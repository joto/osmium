#ifndef OSMIUM_HANDLER_STORE_HPP
#define OSMIUM_HANDLER_STORE_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#include <stdexcept>
#include <cstdlib>
#include <google/sparsetable>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/utility.hpp>

// Darwin uses a different name.
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace Osmium {

    /**
     * @brief Classes handling storage of data.
     */
    namespace Storage {

        /**
         * This abstract class defines an interface to storage classes
         * intended for storing small pieces of data (such as coordinates)
         * indexed by a positive object ID. The storage must be very
         * space efficient and able to scale to billions of objects.
         *
         * Subclasses have different implementations that will store the
         * data in different ways in memory and/or on disk. Some storage
         * classes are better suited when working with the whole planet,
         * some are better for data extracts.
         *
         * Note that these classes are not required to track "empty" fields.
         * When reading data you have to be sure you have put something in
         * there before.
         */
        template <typename TValue>
        class ById : boost::noncopyable {

        public:

            virtual ~ById() {
            }

            /// The "value" type, usually a coordinates class or similar.
            typedef TValue value_type;

            /// Set the field with id to value.
            virtual void set(uint64_t id, TValue value) = 0;

            /// Retrieve value by key. Does not check for overflow or empty fields.
            virtual const TValue& operator[](uint64_t id) const = 0;

            /**
             * Get the approximate number of items in the storage. The storage
             * might allocate memory in blocks, so this size might not be
             * accurate. You can not use this to find out how much memory the
             * storage uses. Use used_memory() for that.
             */
            virtual uint64_t size() const = 0;

            /**
             * Get the memory used for this storage in bytes. Note that this
             * is not necessarily entirely accurate but an approximation.
             * For storage classes that store the data in memory, this is
             * the main memory used, for storage classes storing data on disk
             * this is the memory used on disk.
             */
            virtual uint64_t used_memory() const = 0;

            /**
             * Clear memory used for this storage. After this you can not
             * use the storage container any more.
             */
            virtual void clear() = 0;

        };

        /**
        * The FixedArray storage stores location in a huge array. The size of
        * the array is given when initializing the object, it must be large
        * enough to hold all items.
        *
        * Only use this store when you know beforehand how many IDs there are.
        * It is mainly provided for cases where the more flexible Mmap storage
        * class does not work.
        *
        * There is no range checking on accessing the store.
        *
        * If you are storing node coordinates, you'll need 8 bytes for each node.
        * At the time of writing this, the largest node ID is about 1.3 billion,
        * so you'll need about 10 GB of memory.
        *
        * Note that this storage class will only work on 64 bit systems if
        * used for storing node coordinates. 32 bit systems just can't address
        * that much memory!
        */
        template <typename TValue>
        class FixedArray : public ById<TValue> {

        public:

            /**
             * Constructor.
             *
             * @param max_id One larger than the largest ID you will ever have.
             * @exception std::bad_alloc Thrown when there is not enough memory.
             */
            FixedArray(const uint64_t max_id) : ById<TValue>(), m_size(max_id) {
                m_items = (TValue*) malloc(sizeof(TValue) * max_id);
                if (!m_items) {
                    throw std::bad_alloc();
                }
            }

            ~FixedArray() {
                clear();
            }

            void set(uint64_t id, TValue value) {
                m_items[id] = value;
            }

            const TValue& operator[](uint64_t id) const {
                return m_items[id];
            }

            uint64_t size() const {
                return m_size;
            }

            uint64_t used_memory() const {
                return m_size * sizeof(TValue);
            }

            void clear() {
                free(m_items);
                m_items = NULL;
            }

        private:

            TValue* m_items;

            uint64_t m_size;

        }; // class FixedArray

        /**
        * The SparseTable store stores items in a Google sparsetable,
        * a data structure that can hold sparsly filled tables in a
        * very space efficient way. It will resize automatically.
        *
        * Use this node location store if the ID space is only sparsly
        * populated, such as when working with smaller OSM files (like
        * country extracts).
        */
        template <typename TValue>
        class SparseTable : public ById<TValue> {

        public:

            /**
             * Constructor.
             *
             * @param grow_size The initial size of the storage (in items).
             *                  The storage will grow by at least this size
             *                  every time it runs out of space.
             */
            SparseTable(const uint64_t grow_size=10000)
                : ById<TValue>(),
                  m_grow_size(grow_size),
                  m_items(grow_size) {
            }

            ~SparseTable() {
            }

            void set(uint64_t id, TValue value) {
                if (id >= m_items.size()) {
                    m_items.resize(id + m_grow_size);
                }
                m_items[id] = value;
            }

            const TValue& operator[](uint64_t id) const {
                return m_items[id];
            }

            uint64_t size() const {
                return m_items.size();
            }

            uint64_t used_memory() const {
                // unused items use 1 bit, used items sizeof(TValue) bytes
                // http://google-sparsehash.googlecode.com/svn/trunk/doc/sparsetable.html
                return (m_items.size() / 8) + (m_items.num_nonempty() * sizeof(TValue));
            }

            void clear() {
                m_items.clear();
            }

        private:

            uint64_t m_grow_size;

            google::sparsetable<TValue> m_items;

        }; // class SparseTable

        /**
        * The Mmap store stores location using the mmap() system call,
        * either backed by a file on disk or just in-memory. It will grow
        * automatically.
        *
        * If you have enough memory it is preferred to use the in-memory
        * version. If you don't have enough memory or want the information
        * to persist, use the file-backed version. Note that you still need
        * substantial amounts of memory for this to work efficiently.
        *
        * Note that this storage class will only work on 64 bit systems if
        * used for storing node coordinates. 32 bit systems just can't address
        * that much memory!
        */
        template <typename TValue>
        class Mmap : public ById<TValue> {

        public:

            static const uint64_t size_increment = 10 * 1024 * 1024;

            /**
             * Create anonymous mapping without a backing file.
             * @exception std::bad_alloc Thrown when there is not enough memory.
             */
            Mmap() : ById<TValue>(), m_size(size_increment), m_fd(-1) {
                m_items = (TValue*) mmap(NULL, sizeof(TValue) * m_size, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                if (m_items == MAP_FAILED) {
                    throw std::bad_alloc();
                }
            }

            /**
             * Create mapping backed by file. If filename is empty, a temporary
             * file will be created.
             *
             * @param filename The filename (including the path) for the storage.
             * @param remove Should the file be removed after use?
             * @exception std::bad_alloc Thrown when there is not enough memory or some other problem.
             */
            Mmap(std::string& filename, bool remove=true) : ById<TValue>(), m_size(1) {
                if (filename == "") {
                    FILE* file = tmpfile();
                    if (!file) {
                        throw std::bad_alloc();
                    }
                    m_fd = fileno(file);
                } else {
                    m_fd = open(filename.c_str(), O_RDWR | O_CREAT, 0600);
                }

                if (m_fd < 0) {
                    throw std::bad_alloc();
                }

                // now that the file is open we can immediately remove it
                // (temporary files are always removed)
                if (remove && filename != "") {
                    if (unlink(filename.c_str()) < 0) {
                        // XXX what to do here?
                    }
                }

                // make sure the file is at least as large as the initial size
                if (get_file_size() < sizeof(TValue) * m_size) {
                    if (ftruncate(m_fd, sizeof(TValue) * m_size) < 0) {
                        throw std::bad_alloc();
                    }
                }

                m_items = (TValue*) mmap(NULL, sizeof(TValue) * m_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);
                if (m_items == MAP_FAILED) {
                    throw std::bad_alloc();
                }
            }

            ~Mmap() {
                clear();
            }

            void set(uint64_t id, TValue value) {
                if (id >= m_size) {
                    uint64_t new_size = id + size_increment;

                    // if there is a file backing this mmap and its smaller than needed, increase its size
                    if (m_fd >= 0 && get_file_size() < sizeof(TValue) * new_size) {
                        if (ftruncate(m_fd, sizeof(TValue) * new_size) < 0) {
                            throw std::bad_alloc();
                        }
                    }

                    // use unmap/map instead of remap because Darwin doesn't support remap.
                    munmap(m_items, sizeof(TValue) * m_size);
                    m_items = (TValue*) mmap(NULL, sizeof(TValue) * new_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);
                    if (m_items == MAP_FAILED) {
                        throw std::bad_alloc();
                    }
                    m_size = new_size;
                }
                m_items[id] = value;
            }

            const TValue& operator[](uint64_t id) const {
                return m_items[id];
            }

            uint64_t size() const {
                return m_size;
            }

            uint64_t used_memory() const {
                return m_size * sizeof(TValue);
            }

            void clear() {
                munmap(m_items, sizeof(TValue) * m_size);
            }

        private:

            uint64_t m_size;
            int m_fd;
            TValue* m_items;

            /// Get file size in bytes.
            uint64_t get_file_size() {
                struct stat s;
                if (fstat(m_fd, &s) < 0) {
                    throw std::bad_alloc();
                }
                return s.st_size;
            }

        }; // class Mmap

    } // namespace Storage

} // namespace Osmium

#endif // OSMIUM_HANDLER_STORE_HPP

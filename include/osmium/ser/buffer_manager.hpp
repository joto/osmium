#ifndef OSMIUM_SER_BUFFER_MANAGER_HPP
#define OSMIUM_SER_BUFFER_MANAGER_HPP

/*

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <string>

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>

#include <osmium/ser/buffer.hpp>

namespace Osmium {

    namespace Ser {

        namespace BufferManager {

            class Output : boost::noncopyable {

            public:

                Osmium::Ser::Buffer& output_buffer() {
                    return m_output_buffer;
                }

            protected:
            
                Output(const size_t buffer_size) :
                    m_output_data(buffer_size, '\0'),
                    m_output_buffer(&m_output_data[0], buffer_size, 0) {
                }

                std::string m_output_data;
                Osmium::Ser::Buffer m_output_buffer;

            }; // class Output

            class Memory : public Output {

            public:
            
                Memory(const size_t buffer_size) :
                    Output(buffer_size) {
                }

                void flush_buffer() {
                    // does nothing XXX
                }

                size_t committed() {
                    return m_output_buffer.committed();
                }

                size_t commit() {
                    return m_output_buffer.commit();
                }

                template <class T>
                T& get(const size_t offset) {
                    return m_output_buffer.get<T>(offset);
                }
                
                typedef Osmium::Ser::CollectionIterator<TypedItem> iterator;

                iterator begin() {
                    return m_output_buffer.begin();
                }

                iterator end() {
                    return m_output_buffer.end();
                }

            }; // class Memory

            class FileOutput : public Output {

            public:

                FileOutput(const std::string& output_filename, const size_t buffer_size) :
                    Output(buffer_size),
                    m_output_filename(output_filename),
                    m_output_offset(0),
                    m_output_fd(::open(output_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666)) {
                    if (m_output_fd < 0) {
                        throw std::runtime_error(std::string("Can't open dump file ") + output_filename);
                    }
                }

                void flush_buffer() {
                    ssize_t len = ::write(m_output_fd, m_output_buffer.data(), m_output_buffer.committed());
                    if (len != m_output_buffer.committed()) {
                        throw std::runtime_error(std::string("Can't write to dump file ") + m_output_filename);
                    }
                    m_output_offset += m_output_buffer.clear();
                }

                size_t commit() {
                    return m_output_offset + m_output_buffer.commit();
                }

            protected:

                std::string m_output_filename;
                size_t m_output_offset;
                int m_output_fd;

            }; // class FileOutput

            class FileInput {

            public:

                FileInput(const std::string& input_filename) :
                    m_input_filename(input_filename),
                    m_input_fd(-1),
                    m_input_buffer(NULL) {
                }

                ~FileInput() {
                    cleanup();
                }

                size_t committed() {
                    if (!m_input_buffer) {
                        extend_input_buffer();
                    }
                    return m_input_buffer->committed();
                }

                template <class T>
                T& get(const size_t offset) {
                    if (!m_input_buffer || m_input_buffer->size() <= offset) {
                        extend_input_buffer();
                    }
                    return m_input_buffer->get<T>(offset);
                }
                
                typedef Osmium::Ser::CollectionIterator<TypedItem> iterator;

                iterator begin() {
                    if (!m_input_buffer) {
                        extend_input_buffer();
                    }
                    return m_input_buffer->begin();
                }

                iterator end() {
                    if (!m_input_buffer) {
                        extend_input_buffer();
                    }
                    return m_input_buffer->end();
                }

            private:

                std::string m_input_filename;
                int m_input_fd;
                Osmium::Ser::Buffer* m_input_buffer;

                void cleanup() {
                    if (m_input_buffer) {
                        char* data = m_input_buffer->data();
                        size_t size = m_input_buffer->size();
                        delete m_input_buffer;
                        ::munmap(data, size); 
                    }
                }

                void extend_input_buffer() {
                    if (m_input_fd == -1) {
                        m_input_fd = ::open(m_input_filename.c_str(), O_RDONLY);
                        if (m_input_fd == -1) {
                            throw std::runtime_error(std::string("can't open dump file: ") + strerror(errno));
                        }
                    } else {
                        cleanup();
                    }

                    struct stat file_stat;
                    if (::fstat(m_input_fd, &file_stat) < 0) {
                        throw std::runtime_error(std::string("can't stat dump file: ") + strerror(errno));
                    }

                    size_t bufsize = file_stat.st_size;
                    char* mem = reinterpret_cast<char*>(mmap(NULL, bufsize, PROT_READ, MAP_SHARED, m_input_fd, 0));
                    if (!mem) {
                        throw std::runtime_error(std::string("can't mmap dump file: ") + strerror(errno));
                    }
                    m_input_buffer = new Osmium::Ser::Buffer(mem, bufsize);
                }

            }; // class FileInput

            class File : public FileInput, public FileOutput {

            public:

                File(const std::string& filename, size_t buffer_size) :
                    FileInput(filename),
                    FileOutput(filename, buffer_size) {
                }

            }; // class File

        } // namespace BufferManager

    } // namespace Ser

} // namespace Osmium

#endif // OSMIUM_SER_BUFFER_MANAGER_HPP

#ifndef OSMIUM_OUTPUT_HPP
#define OSMIUM_OUTPUT_HPP

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <map>

#include <osmium/osmfile.hpp>
#include <osmium/handler.hpp>

namespace Osmium {

    /**
     * @brief Classes for writing %OSM files.
     */
    namespace Output {

        class Base : public Osmium::Handler::Base {

        protected:

            Osmium::OSMFile m_file;
            std::string m_generator;

            int fd() {
                return m_file.fd();
            }

        public:

            Base(const Osmium::OSMFile& file) :
                Osmium::Handler::Base(),
                m_file(file),
                m_generator("Osmium (http://wiki.openstreetmap.org/wiki/Osmium)") {
                m_file.open_for_output();
            }

            virtual ~Base() {
            }

            virtual void init(Osmium::OSM::Meta&) = 0;
            virtual void node(const shared_ptr<Osmium::OSM::Node const>&) = 0;
            virtual void way(const shared_ptr<Osmium::OSM::Way const>&) = 0;
            virtual void relation(const shared_ptr<Osmium::OSM::Relation const>&) = 0;
            virtual void final() = 0;

            void set_generator(const std::string& generator) {
                m_generator = generator;
            }

        }; // class Base

        /**
         * This factory class is used to register file output formats and open
         * output files in these formats. You should not use this class directly.
         * Instead use the Osmium::Output::open() function.
         */
        class Factory {

        public:

            typedef Osmium::Output::Base*(*create_output_t)(const Osmium::OSMFile&);

        private:

            typedef std::map<Osmium::OSMFile::FileEncoding*, create_output_t> encoding2create_t;

            Factory() :
                m_callbacks() {}

        public:

            static Factory& instance() {
                static Factory factory;
                return factory;
            }

            bool register_output_format(Osmium::OSMFile::FileEncoding* encoding, create_output_t create_function) {
                return m_callbacks.insert(encoding2create_t::value_type(encoding, create_function)).second;
            }

            bool unregister_output_format(Osmium::OSMFile::FileEncoding* encoding) {
                return m_callbacks.erase(encoding) == 1;
            }

            Osmium::Output::Base* create_output(const Osmium::OSMFile& file) {
                encoding2create_t::iterator it = m_callbacks.find(file.encoding());

                if (it != m_callbacks.end()) {
                    return (it->second)(file);
                }

                throw Osmium::OSMFile::FileEncodingNotSupported();
            }

        private:

            encoding2create_t m_callbacks;

        }; // class Factory

        /**
         */
        class Handler : public Osmium::Handler::Forward<Osmium::Output::Base> {

        public:

            Handler(const Osmium::OSMFile& file) :
                Osmium::Handler::Forward<Osmium::Output::Base>(*Osmium::Output::Factory::instance().create_output(file)) {
            }

            ~Handler() {
                delete &next_handler();
            }

            void set_generator(const std::string& generator) {
                next_handler().set_generator(generator);
            }

        }; // Handler

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_HPP

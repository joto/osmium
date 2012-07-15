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

            int fd() {
                return m_file.fd();
            }

        public:

            Base(const Osmium::OSMFile& file) :
                Osmium::Handler::Base(),
                m_file(file) {
                m_file.open_for_output();
            }

            virtual ~Base() {
            }

            virtual void init(Osmium::OSM::Meta&) = 0;
            virtual void node(const shared_ptr<Osmium::OSM::Node const>&) = 0;
            virtual void way(const shared_ptr<Osmium::OSM::Way const>&) = 0;
            virtual void relation(const shared_ptr<Osmium::OSM::Relation const>&) = 0;
            virtual void final() = 0;

        }; // class Base

    } // namespace Output

} // namespace Osmium

#endif // OSMIUM_OUTPUT_HPP

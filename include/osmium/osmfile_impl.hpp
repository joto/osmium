#ifndef OSMIUM_OSMFILE_IMPL_HPP
#define OSMIUM_OSMFILE_IMPL_HPP

/*

Copyright 2011 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public Licanse and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

namespace Osmium {

    template <class T>
    void OSMFile::read(T* handler) {
        Osmium::Input::Base<T>* input = m_encoding->is_pbf()
                                        ? static_cast<Osmium::Input::Base<T>*>(new Osmium::Input::PBF<T>(*this, handler))
                                        : static_cast<Osmium::Input::Base<T>*>(new Osmium::Input::XML<T>(*this, handler));
        input->parse();
        delete input;
    }

    Osmium::Output::OSM::Base *OSMFile::create_output_file() {
        Osmium::Output::OSM::Base *output;

        if (m_encoding->is_pbf()) {
            output = new Osmium::Output::OSM::PBF(*this);
        } else {
#ifdef OSMIUM_WITH_OUTPUT_OSM_XML
            output = new Osmium::Output::OSM::XML(*this);
#endif
        }

        return output;
    }

} // namespace Osmium

#endif // OSMIUM_OSMFILE_IMPL_HPP

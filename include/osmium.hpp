#ifndef OSMIUM_OSMIUM_HPP
#define OSMIUM_OSMIUM_HPP

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

/**
 * @mainpage
 *
 * Osmium is a C++/Javascript framework for processing OSM files.
 *
 * For more information see http://wiki.openstreetmap.org/wiki/Osmium .
 *
 * The source code is at https://github.com/joto/osmium .
 *
 */

#ifdef OSMIUM_WITH_GEOS
# include <geos/geom/GeometryFactory.h>
#endif // OSMIUM_WITH_GEOS

#ifdef OSMIUM_WITH_JAVASCRIPT
# include <v8.h>
# include <unicode/ustring.h>
# include <osmium/utils/unicode.hpp>
#endif // OSMIUM_WITH_JAVASCRIPT

/**
 * @brief All Osmium code should be in this namespace.
 */
namespace Osmium {

    class Framework;

    struct global {
        Framework *framework;
        bool debug;
#ifdef OSMIUM_WITH_GEOS
        geos::geom::GeometryFactory *geos_geometry_factory;
#endif // OSMIUM_WITH_GEOS
    };

    extern struct global global;

} // namespace Osmium

#ifdef OSMIUM_MAIN
struct Osmium::global Osmium::global;
#endif // OSMIUM_MAIN

// check way geometry before making a shplib object from it
// normally this should be defined, otherwise you will generate invalid linestring geometries
#define OSMIUM_CHECK_WAY_GEOMETRY

#ifdef OSMIUM_WITH_JAVASCRIPT
# include <osmium/javascript/template.hpp>
#endif // OSMIUM_WITH_JAVASCRIPT

#include <osmium/osm.hpp>
#include <osmium/osmfile.hpp>
#include <osmium/input.hpp>
#include <osmium/framework.hpp>
#include <osmium/output.hpp>
#include <osmium/osmfile_impl.hpp>

#ifdef OSMIUM_WITH_JAVASCRIPT
# include <osmium/javascript/object_templates.hpp>
# include <osmium/HandlerJavascript.hpp>
# ifdef OSMIUM_MAIN
namespace Osmium {

    namespace Javascript {

        namespace Template {

            Tags             *js_template_tags;
            NodeGeom         *js_template_nodegeom;
            Node             *js_template_node;
            Nodes            *js_template_nodes;
            WayGeom          *js_template_waygeom;
            Way              *js_template_way;
            Member           *js_template_member;
            Members          *js_template_members;
            Relation         *js_template_relation;
            Multipolygon     *js_template_multipolygon;
            MultipolygonGeom *js_template_multipolygongeom;
            OutputCSV        *js_template_output_csv;
            OutputShapefile  *js_template_output_shapefile;

            v8::Local<v8::Object> create_tags_instance(void *wrapper) {
                return js_template_tags->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_node_geom_instance(void *wrapper) {
                return js_template_nodegeom->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_node_instance(void *wrapper) {
                return js_template_node->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_way_nodes_instance(void *wrapper) {
                return js_template_nodes->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_way_geom_instance(void *wrapper) {
                return js_template_waygeom->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_way_instance(void *wrapper) {
                return js_template_way->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_relation_member_instance(void *wrapper) {
                return js_template_member->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_relation_members_instance(void *wrapper) {
                return js_template_members->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_relation_instance(void *wrapper) {
                return js_template_relation->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_multipolygon_instance(void *wrapper) {
                return js_template_multipolygon->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_multipolygon_geom_instance(void *wrapper) {
                return js_template_multipolygongeom->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_output_csv_instance(void *wrapper) {
                return js_template_output_csv->create_instance(wrapper);
            }

            v8::Local<v8::Object> create_output_shapefile_instance(void *wrapper) {
                return js_template_output_shapefile->create_instance(wrapper);
            }

            void init() {
                js_template_tags             = new Osmium::Javascript::Template::Tags;
                js_template_nodegeom         = new Osmium::Javascript::Template::NodeGeom;
                js_template_node             = new Osmium::Javascript::Template::Node;
                js_template_nodes            = new Osmium::Javascript::Template::Nodes;
                js_template_waygeom          = new Osmium::Javascript::Template::WayGeom;
                js_template_way              = new Osmium::Javascript::Template::Way;
                js_template_member           = new Osmium::Javascript::Template::Member;
                js_template_members          = new Osmium::Javascript::Template::Members;
                js_template_relation         = new Osmium::Javascript::Template::Relation;
                js_template_multipolygon     = new Osmium::Javascript::Template::Multipolygon;
                js_template_multipolygongeom = new Osmium::Javascript::Template::MultipolygonGeom;
                js_template_output_csv       = new Osmium::Javascript::Template::OutputCSV;
                js_template_output_shapefile = new Osmium::Javascript::Template::OutputShapefile;
            }

            void cleanup() {
                delete js_template_output_shapefile;
                delete js_template_output_csv;
                delete js_template_multipolygongeom;
                delete js_template_multipolygon;
                delete js_template_relation;
                delete js_template_members;
                delete js_template_member;
                delete js_template_way;
                delete js_template_waygeom;
                delete js_template_nodes;
                delete js_template_node;
                delete js_template_nodegeom;
                delete js_template_tags;
            }

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium
# endif // OSMIUM_MAIN
#endif // OSMIUM_WITH_JAVASCRIPT

#endif // OSMIUM_OSMIUM_HPP

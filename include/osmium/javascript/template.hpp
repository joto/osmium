#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_HPP

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

#include <v8.h>

namespace Osmium {

    /**
     * @brief %Javascript support for %Osmium.
     */
    namespace Javascript {

        /**
        * Base class for all Javascript template classes. Javascript
        * template classes describe templates from which Javascript
        * objects can be created, so for every C++ class that should
        * be accessible from Javascript there is a corresponding
        * template class.
        *
        * Note that Javascript templates have nothing to do with C++
        * templates.
        */
        class Template {

        public:

            template <class T>
            static T& get() {
                static T t;
                return t;
            }

            /**
             * Create a Javascript object instance from the Javascript template
             * wrapping a C++ object.
             */
            v8::Local<v8::Object> create_instance(void* wrapped) {
                v8::Local<v8::Object> instance = js_template->NewInstance();
                instance->SetInternalField(0, v8::External::New(wrapped));
                return instance;
            }

            template <class TWrapped>
            v8::Persistent<v8::Object> create_persistent_instance(TWrapped* wrapped) {
                v8::Persistent<v8::Object> instance = v8::Persistent<v8::Object>::New(create_instance(wrapped));
                instance.MakeWeak(wrapped, Osmium::Javascript::Template::free_instance<TWrapped>);
                return instance;
            }

            template <class TWrapped>
            static void free_instance(v8::Persistent<v8::Value> instance, void* obj) {
                instance.Dispose();
                delete static_cast<TWrapped*>(obj);
            }

            /**
             * Function that always returns undefined.
             */
            static v8::Handle<v8::Value> undefined(const v8::Arguments& /*args*/) {
                return v8::Undefined();
            }

            /*
                These magic helper function are used to connect Javascript
                methods to C++ methods. They are given to the SetAccessor,
                SetIndexedPropertyHandler and SetNamedPropertyHandler
                functions of a v8::ObjectTemplate object, respectively.

                The first template argument is the class of the object we
                want to access, for instance Osmium::OSM::Node.

                The second template argument is the member function on the
                object that we want to call when this function is called
                from Javascript.

            */
            template <class TWrapped, v8::Handle<v8::Value> (func)(TWrapped*)>
            static v8::Handle<v8::Value> accessor_getter(v8::Local<v8::String>, const v8::AccessorInfo& info) {
                return func(reinterpret_cast<TWrapped*>(v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value()));
            }

            template <class TWrapped, v8::Handle<v8::Value> func(v8::Local<v8::String>, TWrapped*)>
            static v8::Handle<v8::Value> named_property_getter(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
                return func(property, reinterpret_cast<TWrapped*>(v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value()));
            }

            template <class TWrapped, v8::Handle<v8::Value> func(uint32_t, TWrapped*)>
            static v8::Handle<v8::Value> indexed_property_getter(uint32_t index, const v8::AccessorInfo& info) {
                return func(index, reinterpret_cast<TWrapped*>(v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value()));
            }

            template <class TWrapped, v8::Handle<v8::Array> func(TWrapped*)>
            static v8::Handle<v8::Array> property_enumerator(const v8::AccessorInfo& info) {
                return func(reinterpret_cast<TWrapped*>(v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value()));
            }

            template <class TWrapped, v8::Handle<v8::Value> (func)(const v8::Arguments&, TWrapped*)>
            static v8::Handle<v8::Value> function_template(const v8::Arguments& args) {
                return func(args, reinterpret_cast<TWrapped*>(v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value()));
            }

        protected:

            v8::Persistent<v8::ObjectTemplate> js_template;

            Template(int field_count=1) :
                js_template(v8::Persistent<v8::ObjectTemplate>::New(v8::ObjectTemplate::New())) {
                js_template->SetInternalFieldCount(field_count);
            }

            ~Template() {
                js_template.Dispose();
            }

        private:

            // objects of this class can't be copied
            Template(const Template&);
            Template& operator=(const Template&);

        }; // class Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_HPP

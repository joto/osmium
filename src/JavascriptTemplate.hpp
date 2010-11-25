#ifndef OSMIUM_JAVASCRIPT_TEMPLATE_HPP
#define OSMIUM_JAVASCRIPT_TEMPLATE_HPP

namespace Osmium {

    namespace Javascript {

        namespace Template {

            /**
            * Base class for all Javascript templates.
            */
            class Base {

            protected:

                v8::Persistent<v8::ObjectTemplate> js_template;

                Base(int field_count=1) {
                    js_template = v8::Persistent<v8::ObjectTemplate>::New(v8::ObjectTemplate::New());
                    js_template->SetInternalFieldCount(field_count);
                }

                ~Base() {
                    js_template.Dispose();
                }

            public:

                v8::Local<v8::Object> create_instance(void *wrapper) {
                    v8::Local<v8::Object> instance = js_template->NewInstance();
                    instance->SetInternalField(0, v8::External::New(wrapper));
                    return instance;
                }

            }; //class Base

            v8::Local<v8::Object> create_tags_instance(void *wrapper);

            v8::Local<v8::Object> create_node_instance(void *wrapper);

            v8::Local<v8::Object> create_node_geom_instance(void *wrapper);

            v8::Local<v8::Object> create_way_instance(void *wrapper);

            v8::Local<v8::Object> create_way_nodes_instance(void *wrapper);

            v8::Local<v8::Object> create_way_geom_instance(void *wrapper);

            v8::Local<v8::Object> create_relation_member_instance(void *wrapper);

            v8::Local<v8::Object> create_relation_instance(void *wrapper);

            v8::Local<v8::Object> create_relation_members_instance(void *wrapper);

            v8::Local<v8::Object> create_output_csv_instance(void *wrapper);

            v8::Local<v8::Object> create_output_shapefile_instance(void *wrapper);

            void init();

            void cleanup();

        } // namespace Template

    } // namespace Javascript

} // namespace Osmium

#endif // OSMIUM_JAVASCRIPT_TEMPLATE_HPP

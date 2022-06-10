{% for header in include_file_list %}
#include "{{header}}"
{% endfor %}

{% for namespace in namespace_list %}
using {{namespace}};
{% endfor %}

{% for method in method_list %}
{{method.return}} dart_{{class_name_unique}}_{{method.name}}({{class_name}}* handle{% for arg in method.args %},{{arg.type}} {{arg.name}}{% endfor %}) {
    if (handle) {
        return handle->{{method.name}}({% for arg in method.args %}{{arg.name}}{% if not loop.last %},{% endif %}{% endfor %});
    }
    {% if method.defaultreturn %}return {{method.defaultreturn}}; {% endif %}
}
{% endfor %}
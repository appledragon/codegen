{
    "Namespace": "{{ Namespace_placehold }}",
    "Class": "{{ Class_placehold }}",
    "File": "{{File_placehold}}",
    "Methods":[
        {% for method in method_list %}
            {
                "name": "{{method}}",
                "returntype": "{{return_list[loop.index0]}}",
                "keyword": "{{keyword_list[loop.index0]}}",
                "args": [
                        {% for arg in arg_list[loop.index0] %}
                        {
                          "type": "{{arg["type"]}}",
                          "name": "{{arg["name"]}}"
                        }{{ "," if not loop.last }}
                        {% endfor %}
                ]
            }{% if loop.last %}{% else %},{% endif %}
        {% endfor %}
    ]
}
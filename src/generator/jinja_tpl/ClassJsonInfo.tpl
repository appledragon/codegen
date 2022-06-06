{
    "Namespace": "{{ Namespace_placehold }}",
    "Class": "{{ Class_placehold }}",
    "Methods":[
        {% for method in method_list %}
            {
                "name": "{{method}}",
                "returntype": "{{return_list[loop.index0]}}",
                "keyword": "{{keyword_list[loop.index0]}}",
                "args":[
                    
                ]
            }{% if loop.last %}{% else %},{% endif %}
        {% endfor %}
    ]
}
{
    "FileName": "{{ FileName_placehold }}",
    "Class":[
        {% for class in class_declarations_list %}"{{class}}"{% if loop.last %}{% else %},{% endif %}{% endfor %}
    ],
    "Forward Class":[
        {% for class in forward_declarations_list -%}
            "{{class}}"{% if loop.last %}{% else %},{% endif %}
        {%endfor -%}
    ]
}
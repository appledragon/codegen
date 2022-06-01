/**
 * auto generated, DO NOT EDIT THIS FILE
 */

#include "Mock{{ service_name }}.h"
#include "services/{{ service_name }}/{{ service_name }}.h"
#include "services/IVislaServiceCore.h"
#include "mock/adapters/Mock{{ adapter_name }}.h"

using namespace vcf;

Mock{{ service_name }}::Mock{{ service_name }}(const weakHandle<IVislaServiceCore> &vislaServiceCore) :
    mVislaServiceCore(vislaServiceCore)
{
    real{{ service_name }} = std::make_shared<{{ service_name }}>(vislaServiceCore);

{% for method in method_list %}
	ON_CALL(*this, {{method}}({{ arg_list[loop.index0] }}))
		.WillByDefault(::Invoke(dynamic_cast<{{ service_name }} *>(real{{ service_name }}.get()), &{{ service_name }}::{{method}}));
{% endfor %}

}

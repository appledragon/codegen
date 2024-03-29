/**
 * auto generated, DO NOT EDIT THIS FILE
 */

#include "Mock{{ adapter_name }}.h"
#include "adapters/{{ adapter_name }}/{{ adapter_name }}.h"


using namespace vcf;

Mock{{ adapter_name }}::Mock{{ adapter_name }}(const vcf::weakHandle<INetworkManager>& networkManager) :
    mNetworkManager(networkManager)
{
    real{{ adapter_name }} = I{{ adapter_name }}::CreateInstance(networkManager);

{% for method in method_list %}
	ON_CALL(*this, {{method}}({{ arg_list[loop.index0] }}))
		.WillByDefault(::Invoke(dynamic_cast<{{ adapter_name }} *>(real{{ adapter_name }}.get()), &{{ adapter_name }}::{{method}}));
{% endfor %}

}


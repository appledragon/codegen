#pragma once
#include <gmock/gmock.h>

#include "adapters/I{{ class_name }}.h"

using namespace vcf;
using ::testing::_;
using ::testing::Invoke;

class Mock{{ class_name }} : public I{{ class_name }}
{
public:
    Mock{{ class_name }}(const vcf::weakHandle<vcf::INetworkEventHandler>& networkEventHandler);
    ~Mock{{ class_name }}() override = default;

    static std::shared_ptr<I{{ class_name }}> CreateMockInstance(const vcf::weakHandle<vcf::INetworkEventHandler>& networkEventHandler)
    {
        return std::make_shared<Mock{{ class_name }}>(networkEventHandler);
    }

{% for method in method_list %}
    MOCK_METHOD({{ return_list[loop.index0] }}, 
                {{method}}, 
                ({% for arg in arg_list[loop.index0][method] %}{{arg}}{{ "," if not loop.last }}{% endfor %}), 
                ({{ keyword_list[loop.index0] }}));
{% endfor %}

private:
    std::shared_ptr<I{{ class_name }}> real{{ class_name }};
    vcf::weakHandle<vcf::INetworkEventHandler> mNetworkEventHandler;
};

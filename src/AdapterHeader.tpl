#pragma once
#include <gmock/gmock.h>

#include "adapters/IUserAdapter.h"

using namespace vcf;
using ::testing::_;
using ::testing::Invoke;

class Mock{{ adapter_name }} : public I{{ adapter_name }}
{
public:
    ~Mock{{ adapter_name }}() override = default;
    explicit Mock{{ adapter_name }}(const vcf::weakHandle<INetworkManager>& networkManager);

    static std::shared_ptr<I{{ adapter_name }}> CreateMockInstance(const vcf::weakHandle<INetworkManager>& networkManager)
    {
        return std::make_shared<Mock{{ adapter_name }}>(networkManager);
    }

{% for method in method_list %}
    MOCK_METHOD({{ return_list[loop.index0] }}, 
                {{method}}, 
                ({% for arg in arg_list[loop.index0][method] %}{{arg}}{{ "," if not loop.last }}{% endfor %}), 
                ({{ keyword_list[loop.index0] }}));
{% endfor %}

private:
    std::shared_ptr<I{{ adapter_name }}> real{{ adapter_name }};
    vcf::weakHandle<vcf::INetworkManager> mNetworkManager;
};

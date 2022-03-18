#pragma once
#include <gmock/gmock.h>

#include "adapters/IUserAdapter.h"

using namespace vcf;
using ::testing::_;
using ::testing::Invoke;

class MockUserAdapter : public IUserAdapter
{
public:
    ~MockUserAdapter() override = default;
    explicit MockUserAdapter(const vcf::weakHandle<INetworkManager>& networkManager);

    static std::shared_ptr<IUserAdapter> CreateMockInstance(const vcf::weakHandle<INetworkManager>& networkManager)
    {
        return std::make_shared<MockUserAdapter>(networkManager);
    }

            // GET /v1/user/get-upload-url get user's file upload url
    MOCK_METHOD( void ,getUploadUrl,(const std::string& fileSuffix,
                              const model::UploadResourceType& type,
                 const internalCallback::innerUploadUrlCallback& callback),
                (override));

    // PUT /v1/user/avatar/edit update user's avatar
    MOCK_METHOD( void ,updateAvatar,(const std::string& avatarUrl,
                              const std::string& resourceUuid,
                 const internalCallback::simpleStringCallback& callback),(override));

    // POST /v1/user/device/register device register
    MOCK_METHOD( void ,registerDevice,(const vcf::model::DeviceInfo& deviceInfo,
                                const std::string& deviceToken,
                 const internalCallback::simpleCallback& callback),
  

private:

    std::shared_ptr<IUserAdapter> realUserAdapter;
};

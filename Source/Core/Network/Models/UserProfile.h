/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "ApiModel.h"

class UserProfile final : public ApiModel
{
public:

    UserProfile(const ValueTree &tree, const Image image = {}) : ApiModel(tree), avatar(image)
    {
        using namespace Serialization::Api::V1;
        const bool hasImage = this->avatar.isValid();
        const bool hasCache = this->data.hasProperty(Identity::avatarThumbnail);
        if (hasImage && !hasCache)
        {
            MemoryBlock block;
            {
                MemoryOutputStream outStream(block, false);
                this->imageFormat.writeImageToStream(this->avatar, outStream);
            }

            const auto avatarData(Base64::toBase64(block.getData(), block.getSize()));
            this->data.setProperty(Identity::avatarThumbnail, avatarData, nullptr);
        }
    }

    String getEmail() const noexcept
    { return this->data.getProperty(Serialization::Api::V1::Identity::email); }

    String getLogin() const noexcept
    { return this->data.getProperty(Serialization::Api::V1::Identity::login); }

    String getName() const noexcept
    { return this->data.getProperty(Serialization::Api::V1::Identity::name); }

    String getProfileUrl() const noexcept
    { return this->data.getProperty(Serialization::Api::V1::Identity::profileUrl); }

    String getAvatarUrl() const noexcept
    { return this->data.getProperty(Serialization::Api::V1::Identity::avatarUrl); }

    Image getAvatar() const noexcept
    { return this->avatar; }

    void deserialize(const ValueTree &tree) override
    {
        ApiModel::deserialize(tree);

        using namespace Serialization::Api::V1;
        if (this->data.hasProperty(Identity::avatarThumbnail))
        {
            MemoryBlock block;
            {
                MemoryOutputStream outStream(block, false);
                Base64::convertFromBase64(outStream, { this->data.getProperty(Identity::avatarThumbnail) });
            }

            this->avatar = ImageFileFormat::loadFrom(block.getData(), block.getSize());
        }
    }

private:

    Image avatar;
    PNGImageFormat imageFormat;

    JUCE_LEAK_DETECTOR(UserProfile)
};

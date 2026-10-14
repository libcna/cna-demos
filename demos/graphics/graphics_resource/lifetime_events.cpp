// SPDX-License-Identifier: MS-PL
// GraphicsResource lifetime: ResourceCreated/ResourceDestroyed on the device, Disposing, Name and Tag.
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/ResourceCreatedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Graphics/ResourceDestroyedEventArgs.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/EventArgs.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A Tag is any System::Object. Here: the game object that owns the texture.
    class PortraitOwner final : public System::Object
    {
    public:
        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string name = "PortraitOwner";
            return name;
        }
    };

    class LifetimeEventsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // The device reports every resource created on it, and every resource destroyed.
            // ResourceDestroyed carries the Name and Tag, not the object: it may be gone already.
            created_ = device.ResourceCreated.Add([this](System::Object*, const ResourceCreatedEventArgs& e) {
                ++createdCount_;
                lastCreated_ = e.getResourceProperty();
            });
            destroyed_ = device.ResourceDestroyed.Add([this](System::Object*, const ResourceDestroyedEventArgs& e) {
                ++destroyedCount_;
                destroyedName_ = e.getNameProperty();
                destroyedTag_ = e.getTagProperty();
            });

            {
                Texture2D texture(device, 16, 16);
                Context().Check(createdCount_ == 1 && lastCreated_ == &texture,
                                "ResourceCreated fired for the new texture, with the texture as Resource");
                Context().Check(texture.getGraphicsDeviceProperty() == &device,
                                "GraphicsDevice identifies the device that owns the resource");
                Context().Check(texture.ToString() == texture.GetTypeName(),
                                "an unnamed resource formats as its runtime type name");

                // Name and Tag are free for the game: a debug name, and any object to find the
                // owner again (XNA's Tag is an object reference; in CNA a System::Object*).
                texture.setNameProperty("player-portrait");
                texture.setTagProperty(&owner_);
                Context().Check(texture.ToString() == "player-portrait",
                                "a named resource formats as its debug name");

                bool disposing = false;
                texture.Disposing += [&disposing](System::Object*, const System::EventArgs&) { disposing = true; };

                texture.Dispose();
                Context().Check(disposing, "Dispose raised the resource's own Disposing event");
                Context().Check(texture.getIsDisposedProperty(), "IsDisposed is true after Dispose");
                Context().Check(destroyedCount_ == 1, "ResourceDestroyed fired once");
                Context().Check(destroyedName_ == "player-portrait", "ResourceDestroyed carries the Name");
                Context().Check(destroyedTag_ == &owner_, "ResourceDestroyed carries the Tag");

                // Disposing twice is allowed and does nothing.
                texture.Dispose();
                Context().Check(destroyedCount_ == 1, "a second Dispose raises nothing");
            }
            Context().Print("created {}, destroyed {} (last destroyed: \"{}\")", createdCount_, destroyedCount_,
                            destroyedName_);
        }

        void UnloadContent() override
        {
            // The handlers capture this game; remove them before the device outlives it.
            getGraphicsDeviceProperty().ResourceCreated.Remove(created_);
            getGraphicsDeviceProperty().ResourceDestroyed.Remove(destroyed_);
        }

    private:
        PortraitOwner owner_;
        System::EventHandler<ResourceCreatedEventArgs>::Token created_{};
        System::EventHandler<ResourceDestroyedEventArgs>::Token destroyed_{};
        int createdCount_ = 0;
        int destroyedCount_ = 0;
        System::Object* lastCreated_ = nullptr;
        std::string destroyedName_;
        System::Object* destroyedTag_ = nullptr;
    };
}

CNA_DEMOS_GAME(LifetimeEventsDemo, {
    .id = "graphics.graphics_resource.lifetime_events",
    .title = "Resource lifetime events",
    .description = "Watches GraphicsDevice::ResourceCreated and ResourceDestroyed while a texture with a Name and Tag "
                   "is created and disposed, and the texture's own Disposing event.",
    .apis = {"GraphicsDevice::ResourceCreated", "GraphicsDevice::ResourceDestroyed", "ResourceCreatedEventArgs::Resource",
             "ResourceDestroyedEventArgs::Name", "ResourceDestroyedEventArgs::Tag", "GraphicsResource::Name",
             "GraphicsResource::Tag", "GraphicsResource::GraphicsDevice", "GraphicsResource::ToString",
             "GraphicsResource::Disposing", "GraphicsResource::IsDisposed",
             "GraphicsResource::Dispose"},
    .tags = {"resource", "dispose", "event", "lifetime"},
    .frames = 1,
})

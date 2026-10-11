// SPDX-License-Identifier: MS-PL
#pragma once
// Shared set-up of the demos that use gamer services. Header-only: only the categories that link
// CNA_GamerServices (gamer_services, net) include it.

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#include "CNA/GamerServices/Configuration.hpp"
#include "CnaDemos/DemoGame.hpp"
#include "CnaDemos/Metadata.hpp" // DemoEntry, for the ownProcess check
#include "Microsoft/Xna/Framework/GamerServices/GamerServicesComponent.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"

namespace CnaDemos
{
    /** The local profile the demos sign in, unless CNA_GAMER_SERVICES_AUTO_SIGN_IN names others. */
    inline constexpr const char* DemoGamertag = "Demo Player";

    /** Sets an environment variable for this process and the processes it starts. */
    inline void SetEnvironment(const char* name, const std::string& value)
    {
#ifdef _WIN32
        _putenv_s(name, value.c_str());
#else
        setenv(name, value.c_str(), 1);
#endif
    }

    /** Sets an environment variable unless the user already set it. */
    inline void SetEnvironmentDefault(const char* name, const std::string& value)
    {
        const char* current = std::getenv(name);
        if (current == nullptr || *current == '\0')
        {
            SetEnvironment(name, value);
        }
    }

    /**
     * CNA's offline gamer services with the demos' own profile store (see GamerServicesDemo
     * below). The standalone net programs, which are not DemoGames, call it themselves before
     * adding their GamerServicesComponent.
     */
    inline void UseOfflineDemoProfiles()
    {
        CNA::GamerServices::setConfigurationOverride(CNA::GamerServices::Configuration{});
        Microsoft::Xna::Framework::Storage::StorageDevice::SetAppNameEXT("cna-demos");
        SetEnvironmentDefault(
            "CNA_GAMER_SERVICES_PROFILES_DIR",
            (std::filesystem::path(Microsoft::Xna::Framework::Storage::StorageDevice::GetStorageRootEXT()) / "profiles")
                .string());
    }

    /**
     * Base class of the demos about gamer services. Its constructor does what an XNA game's
     * constructor does -- adds a GamerServicesComponent to Components -- after three settings
     * that keep the demos self-contained:
     *   - CNA's offline mode (local profiles, no CNA service) through
     *     CNA::GamerServices::setConfigurationOverride, so a CNA service configured for the
     *     user's own games (CNA_GAMER_SERVICES_ENDPOINT, cna-title.json,
     *     ~/.config/cna/gamer-services.json) is never contacted: the demos open no connection;
     *   - the demos' profiles are kept apart from the user's own CNA profiles: unless
     *     CNA_GAMER_SERVICES_PROFILES_DIR is set, they live in "profiles" in the demos' storage
     *     root (StorageDevice::GetStorageRootEXT after SetAppNameEXT("cna-demos"));
     *   - the local profile "Demo Player" is signed in when the game starts, as a console signs
     *     in its default profile (CNA_GAMER_SERVICES_AUTO_SIGN_IN, unless already set; CNA
     *     creates the profile the first time). A demo with several local players passes their
     *     comma-separated names instead (at most four, players One to Four in order).
     * Achievements and leaderboards CNA keeps offline go to the same storage root.
     *
     * Gamer services are initialized once per process, so every demo built on this class sets
     * DemoInfo::ownProcess; the constructor refuses to run otherwise.
     */
    class GamerServicesDemo : public DemoGame
    {
    public:
        explicit GamerServicesDemo(const char* autoSignIn = DemoGamertag)
        {
            if (!Context().Entry().info.ownProcess)
            {
                throw std::logic_error("a gamer services demo must set .ownProcess = true in its metadata");
            }
            UseOfflineDemoProfiles();
            SetEnvironmentDefault("CNA_GAMER_SERVICES_AUTO_SIGN_IN", autoSignIn);

            // The component initializes GamerServicesDispatcher in Initialize, updates it every
            // frame and draws the Guide over the game. The collection keeps it alive.
            getComponentsProperty().Add(
                std::make_shared<Microsoft::Xna::Framework::GamerServices::GamerServicesComponent>(*this));
        }
    };
}

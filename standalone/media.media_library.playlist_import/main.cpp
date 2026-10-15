// SPDX-License-Identifier: MS-PL
// Import public Playlist/Song views from isolated M3U fixtures, including containment boundaries.
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/Media/MediaLibrary.hpp"
#include "Microsoft/Xna/Framework/Media/Playlist.hpp"
#include "Microsoft/Xna/Framework/Media/PlaylistCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/Media/SongCollection.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
#if defined(__linux__)
    struct PrivateLibrary
    {
        std::filesystem::path path;
        ~PrivateLibrary()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    void CopyAsset(const std::string& asset, const std::filesystem::path& destination)
    {
        std::unique_ptr<System::IO::Stream> input(TitleContainer::OpenStream(asset));
        const auto length = input->getLengthProperty();
        if (length <= 0 || length > 1024 * 1024)
        {
            throw std::runtime_error("Unexpected music fixture size");
        }
        std::vector<SharpRuntime::bytecs> bytes(static_cast<std::size_t>(length));
        SharpRuntime::intcs offset = 0;
        while (offset < length)
        {
            const auto count = input->Read(bytes.data(), offset, length - offset);
            if (count <= 0) throw std::runtime_error("Music fixture stream ended early");
            offset += count;
        }
        std::ofstream output(destination, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        output.close();
        if (!output) throw std::runtime_error("Could not copy music fixture");
    }

    Playlist* FindPlaylist(PlaylistCollection* playlists, const std::string& name)
    {
        if (!playlists) return nullptr;
        for (Playlist* playlist : *playlists)
        {
            if (playlist && playlist->getNameProperty() == name) return playlist;
        }
        return nullptr;
    }
#endif

    int PlaylistImport()
    {
#if !defined(__linux__)
        std::cout << "SKIPPED: requires Linux XDG user-folder resolution\n";
        return 77;
#else
        bool passed = true;
        int checks = 0;
        const auto check = [&](bool condition, std::string_view message) {
            ++checks;
            passed = passed && condition;
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            return condition;
        };

        const auto root = std::filesystem::temp_directory_path() /
            ("cna-demos-playlist-import-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root))
            throw std::runtime_error("Could not create private media directory");
        PrivateLibrary cleanup{root};
        const auto music = root / "music";
        const auto pictures = root / "pictures";
        const auto config = root / "config";
        const auto outside = root / "outside.flac";
        std::filesystem::create_directories(music / "sub");
        std::filesystem::create_directory(pictures);
        std::filesystem::create_directory(config);
        if (root.string().find_first_of("\"\\\n") != std::string::npos)
            throw std::runtime_error("Temporary path cannot be represented in this XDG fixture");
        {
            std::ofstream settings(config / "user-dirs.dirs");
            settings << "XDG_MUSIC_DIR=\"" << music.string() << "\"\n"
                     << "XDG_PICTURES_DIR=\"" << pictures.string() << "\"\n";
            settings.close();
            if (!settings) throw std::runtime_error("Could not write private XDG configuration");
        }
        if (::setenv("XDG_CONFIG_HOME", config.c_str(), 1) != 0)
            throw std::runtime_error("Could not select private XDG configuration");

        auto* filesystem = CNA::Platform::GetCurrentPlatform().GetFileSystem();
        std::error_code error;
        const bool musicIsPrivate = filesystem && std::filesystem::equivalent(
            filesystem->GetUserFolder(CNA::Platform::UserFolder::Music), music, error);
        error.clear();
        const bool picturesArePrivate = filesystem && std::filesystem::equivalent(
            filesystem->GetUserFolder(CNA::Platform::UserFolder::Pictures), pictures, error);
        if (!musicIsPrivate || !picturesArePrivate)
        {
            std::cout << "SKIPPED: platform does not honor the isolated Linux XDG media folders\n";
            return 77;
        }
        check(musicIsPrivate && picturesArePrivate,
              "both media roots resolve to private fixture directories before library access");

        CopyAsset("assets/media/library_music/track_a.flac", music / "a.flac");
        CopyAsset("assets/media/library_music/track_b.flac", music / "sub" / "b.flac");
        CopyAsset("assets/media/library_music/track_c.flac", music / "\xc3\x89" "cho.flac");
        CopyAsset("assets/media/library_music/track_c.flac", outside);
        std::filesystem::create_symlink(outside, music / "escape.flac", error);
        if (error) throw std::runtime_error("Could not create playlist containment symlink");

        {
            std::ofstream playlist(music / "Boundaries.m3u", std::ios::binary);
            playlist << "#EXTM3U\r\n#EXTINF:1,Dawn\r\n  a.flac  \r\nsub/b.flac\r\n./a.flac\r\nmissing.flac\r\n";
        }
        {
            std::ofstream playlist(music / "Absolute.m3u", std::ios::binary);
            playlist << (music / "a.flac").string() << '\n';
        }
        {
            std::ofstream playlist(music / "Symlink.m3u", std::ios::binary);
            playlist << "escape.flac\n";
        }
        {
            std::ofstream playlist(music / "Unicode.M3U8", std::ios::binary);
            playlist << "#EXTM3U\r\n\xc3\x89" "cho.flac\r\n";
        }
        {
            std::ofstream unsupported(music / "Ignored.pls");
            unsupported << "[playlist]\nFile1=a.flac\n";
            std::ofstream nested(music / "sub" / "Nested.m3u");
            nested << "b.flac\n";
        }

        MediaLibrary library;
        auto* playlists = library.getPlaylistsProperty();
        if (!check(playlists && playlists->getCountProperty() == 4,
                   "only four root-level M3U/M3U8 files become public playlists")) return 1;
        Playlist* boundaries = FindPlaylist(playlists, "Boundaries");
        Playlist* absolute = FindPlaylist(playlists, "Absolute");
        Playlist* symlink = FindPlaylist(playlists, "Symlink");
        Playlist* unicode = FindPlaylist(playlists, "Unicode");
        if (!check(boundaries && absolute && symlink && unicode &&
                       !FindPlaylist(playlists, "Ignored") && !FindPlaylist(playlists, "Nested"),
                   "playlist names come from supported root-level filenames only")) return 1;

        SongCollection* boundarySongs = boundaries->getSongsProperty();
        if (!check(boundarySongs && boundarySongs->getCountProperty() == 3,
                   "comments and a missing entry are ignored while three valid entries remain")) return 1;
        Song* first = (*boundarySongs)[0];
        Song* second = (*boundarySongs)[1];
        Song* third = (*boundarySongs)[2];
        check(first && second && third && first->getNameProperty() == "Dawn" &&
                  second->getNameProperty() == "Dusk" && third->getNameProperty() == "Dawn",
              "trimmed and nested relative entries preserve authored playlist order");
        check(first == third,
              "a duplicate M3U entry is retained as the same library-owned Song object");
        check(absolute->getSongsProperty() && absolute->getSongsProperty()->getCountProperty() == 0,
              "an absolute entry is rejected even when it names an indexed song");
        check(symlink->getSongsProperty() && symlink->getSongsProperty()->getCountProperty() == 0,
              "an in-root symlink whose target escapes the playlist directory is rejected");
        SongCollection* unicodeSongs = unicode->getSongsProperty();
        check(unicodeSongs && unicodeSongs->getCountProperty() == 1 && (*unicodeSongs)[0] &&
                  (*unicodeSongs)[0]->getNameProperty() == "Echo",
              "uppercase M3U8 extension, UTF-8 filename and CRLF input round-trip together");

        std::cout << (passed ? "PASSED" : "FAILED") << " (" << checks << " checks)\n";
        return passed ? 0 : 1;
#endif
    }
}

int main()
{
    try { return PlaylistImport(); }
    catch (const std::exception& error)
    {
        std::cerr << "Playlist import demo failed: " << error.what() << '\n';
        return 1;
    }
}

// SPDX-License-Identifier: MS-PL
// Index every supported MediaLibrary audio extension through public Song/group views.
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/Media/Album.hpp"
#include "Microsoft/Xna/Framework/Media/AlbumCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Artist.hpp"
#include "Microsoft/Xna/Framework/Media/ArtistCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Genre.hpp"
#include "Microsoft/Xna/Framework/Media/GenreCollection.hpp"
#include "Microsoft/Xna/Framework/Media/MediaLibrary.hpp"
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
            throw std::runtime_error("Unexpected audio fixture size");
        std::vector<SharpRuntime::bytecs> bytes(static_cast<std::size_t>(length));
        SharpRuntime::intcs offset = 0;
        while (offset < length)
        {
            const auto count = input->Read(bytes.data(), offset, length - offset);
            if (count <= 0) throw std::runtime_error("Audio fixture stream ended early");
            offset += count;
        }
        std::ofstream output(destination, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
        output.close();
        if (!output) throw std::runtime_error("Could not copy audio fixture");
    }
#endif

    int FormatIndex()
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
            ("cna-demos-format-index-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root))
            throw std::runtime_error("Could not create private media directory");
        PrivateLibrary cleanup{root};
        const auto music = root / "music";
        const auto pictures = root / "pictures";
        const auto config = root / "config";
        const auto tagged = music / "Tagged" / "Containers";
        const auto fallback = music / "Fallback Artist" / "Fallback Album";
        const auto partial = music / "Partial Artist" / "Partial Album";
        const auto linkedLibrary = root / "linked-library";
        std::filesystem::create_directories(tagged);
        std::filesystem::create_directories(fallback);
        std::filesystem::create_directories(partial);
        std::filesystem::create_directory(linkedLibrary);
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

        const std::array<std::pair<const char*, const char*>, 5> taggedAssets{{
            {"format_vorbis.ogg", "01-vorbis.ogg"},
            {"format_alias.oga", "02-alias.OGA"},
            {"format_id3.mp3", "03-id3.MP3"},
            {"format_flac.flac", "04-flac.flac"},
            {"format_opus.opus", "05-opus.OpUs"},
        }};
        for (const auto& [source, destination] : taggedAssets)
        {
            CopyAsset(std::string("assets/media/library_formats/") + source, tagged / destination);
        }
        CopyAsset("assets/music/rising.wav", fallback / "Plain Wave.WAV");
        CopyAsset("assets/media/library_formats/format_partial.ogg", partial / "06-partial.ogg");
        CopyAsset("assets/media/library_music/track_c.flac", linkedLibrary / "linked.flac");
        CopyAsset("assets/media/library_formats/format_vorbis.ogg", music / "ignored.m4a");
        CopyAsset("assets/media/library_formats/format_opus.opus", music / "ignored.aac");
        CopyAsset("assets/media/library_formats/format_flac.flac", music / "ignored.bin");
        std::filesystem::create_directory_symlink(music, music / "zz-cycle", error);
        if (error) throw std::runtime_error("Could not create recursive library symlink");
        error.clear();
        std::filesystem::create_directory_symlink(linkedLibrary, music / "Linked External", error);
        if (error) throw std::runtime_error("Could not create external library symlink");

        MediaLibrary library;
        SongCollection* songs = library.getSongsProperty();
        if (!check(songs && songs->getCountProperty() == 8,
                   "recursive scanning indexes supported files once and ignores AAC, M4A and BIN")) return 1;
        std::map<std::string, Song*> byName;
        for (Song* song : *songs)
        {
            if (song) byName.emplace(song->getNameProperty(), song);
        }
        const std::array<std::string, 8> expectedNames{
            "Vorbis Song", "OGA Alias", "ID3 Song", "FLAC Song", "Opus Song", "Plain Wave",
            "Partial Tag", "Echo"};
        bool hasAllNames = byName.size() == expectedNames.size();
        for (const auto& name : expectedNames) hasAllNames = hasAllNames && byName.contains(name);
        if (!check(hasAllNames,
                   "all six extension routes plus partial-tag and linked records expose expected Songs")) return 1;

        const std::array<std::string, 5> taggedNames{
            "Vorbis Song", "OGA Alias", "ID3 Song", "FLAC Song", "Opus Song"};
        Artist* sharedArtist = byName[taggedNames[0]]->getArtistProperty();
        Album* sharedAlbum = byName[taggedNames[0]]->getAlbumProperty();
        Genre* sharedGenre = byName[taggedNames[0]]->getGenreProperty();
        bool tagsAgree = sharedArtist && sharedAlbum && sharedGenre &&
            sharedArtist->getNameProperty() == "Format Ensemble" &&
            sharedAlbum->getNameProperty() == "Format Collection" &&
            sharedGenre->getNameProperty() == "Signals";
        for (std::size_t i = 0; i < taggedNames.size(); ++i)
        {
            Song* song = byName[taggedNames[i]];
            tagsAgree = tagsAgree && song->getTrackNumberProperty() == static_cast<int>(i + 1) &&
                song->getArtistProperty() == sharedArtist && song->getAlbumProperty() == sharedAlbum &&
                song->getGenreProperty() == sharedGenre;
        }
        check(tagsAgree,
              "all four tag layouts preserve titles/tracks and share normalized artist, album and genre objects");
        check(byName["OGA Alias"]->getArtistProperty() == sharedArtist,
              "case-variant artist tags normalize to the first deterministic display value");
        check(byName["Vorbis Song"]->getIsRatedProperty() &&
                  byName["Vorbis Song"]->getRatingProperty() == 8 &&
                  !byName["OGA Alias"]->getIsRatedProperty() && byName["OGA Alias"]->getRatingProperty() == 0,
              "a Vorbis rating reaches Song as 8 while an unrated peer stays explicitly unrated");

        Song* plain = byName["Plain Wave"];
        check(plain->getArtistProperty() && plain->getAlbumProperty() && !plain->getGenreProperty() &&
                  plain->getArtistProperty()->getNameProperty() == "Fallback Artist" &&
                  plain->getAlbumProperty()->getNameProperty() == "Fallback Album" &&
                  plain->getTrackNumberProperty() == 0 && !plain->getIsRatedProperty() &&
                  plain->getRatingProperty() == 0,
              "an untagged WAV derives title, artist and album from its nested path without inventing other tags");
        Song* partialSong = byName["Partial Tag"];
        check(partialSong->getArtistProperty() && partialSong->getAlbumProperty() &&
                  !partialSong->getGenreProperty() && partialSong->getNameProperty() == "Partial Tag" &&
                  partialSong->getArtistProperty()->getNameProperty() == "Partial Artist" &&
                  partialSong->getAlbumProperty()->getNameProperty() == "Partial Album" &&
                  partialSong->getTrackNumberProperty() == 0,
              "a real title tag is retained while absent artist and album tags fall back to the nested path");
        Song* linkedSong = byName["Echo"];
        check(linkedSong->getArtistProperty() && linkedSong->getAlbumProperty() && linkedSong->getGenreProperty() &&
                  linkedSong->getArtistProperty()->getNameProperty() == "Second Ensemble" &&
                  linkedSong->getAlbumProperty()->getNameProperty() == "Field Notes",
              "a directory symlink outside the Music root is followed once and preserves real tags");
        check(library.getArtistsProperty()->getCountProperty() == 4 &&
                  library.getAlbumsProperty()->getCountProperty() == 4 &&
                  library.getGenresProperty()->getCountProperty() == 2,
              "full, partial, fallback and linked records form four artists, four albums and two genres");
#ifdef CNA_FFMPEG_AVAILABLE
        bool durationsPositive = true;
        for (Song* song : *songs)
            durationsPositive = durationsPositive && song->getDurationProperty().getTicksProperty() > 0;
        check(durationsPositive,
              "all eight indexed records expose a positive import-time duration without playback");
#else
        std::cout << "Duration probing not exercised: CNA was built without FFmpeg metadata support\n";
#endif

        std::cout << (passed ? "PASSED" : "FAILED") << " (" << checks << " checks)\n";
        return passed ? 0 : 1;
#endif
    }
}

int main()
{
    try { return FormatIndex(); }
    catch (const std::exception& error)
    {
        std::cerr << "Music format index demo failed: " << error.what() << '\n';
        return 1;
    }
}

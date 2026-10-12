// SPDX-License-Identifier: MS-PL
// Import tagged songs and follow library-owned album, artist, genre and playlist relationships.
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/Media/Album.hpp"
#include "Microsoft/Xna/Framework/Media/AlbumCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Artist.hpp"
#include "Microsoft/Xna/Framework/Media/ArtistCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Genre.hpp"
#include "Microsoft/Xna/Framework/Media/GenreCollection.hpp"
#include "Microsoft/Xna/Framework/Media/MediaLibrary.hpp"
#include "Microsoft/Xna/Framework/Media/Playlist.hpp"
#include "Microsoft/Xna/Framework/Media/PlaylistCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "Microsoft/Xna/Framework/Media/SongCollection.hpp"
#include "System/IO/Stream.hpp"

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
#endif
    int MusicIndex()
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
        // Configure only this child process, before the first CNA/SDL call. HOME is unchanged.
        const auto root = std::filesystem::temp_directory_path() /
            ("cna-demos-music-index-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root)) throw std::runtime_error("Could not create private media directory");
        PrivateLibrary cleanup{root};
        const auto music = root / "music", pictures = root / "pictures", config = root / "config";
        std::filesystem::create_directory(music);
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
        check(musicIsPrivate && picturesArePrivate, "both media roots resolve to private fixture directories before library access");

        // Basenames and folders differ from the authored tags: successful grouping must use tags.
        for (const char* name : {"track_a.flac", "track_b.flac", "track_c.flac"})
        {
            auto input = TitleContainer::OpenStream(std::string("assets/media/library_music/") + name);
            const auto length = input->getLengthProperty();
            if (length <= 0 || length > 1024 * 1024) throw std::runtime_error("Unexpected fixture size");
            std::vector<SharpRuntime::bytecs> bytes(static_cast<std::size_t>(length));
            SharpRuntime::intcs offset = 0;
            while (offset < length)
            {
                const auto count = input->Read(bytes.data(), offset, length - offset);
                if (count <= 0) throw std::runtime_error("Music fixture stream ended early");
                offset += count;
            }
            std::ofstream output(music / name, std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            output.close();
            if (!output) throw std::runtime_error("Could not copy music fixture");
        }
        {
            std::ofstream playlist(music / "Selected.m3u");
            playlist << "#EXTM3U\ntrack_b.flac\ntrack_a.flac\n";
            playlist.close();
            if (!playlist) throw std::runtime_error("Could not write private playlist");
        }
        MediaLibrary library;
        // Every pointer below is borrowed from library, including the group's collection views.
        auto* songs = library.getSongsProperty();
        auto* albums = library.getAlbumsProperty();
        auto* artists = library.getArtistsProperty();
        auto* genres = library.getGenresProperty();
        auto* playlists = library.getPlaylistsProperty();
        if (!check(songs && albums && artists && genres && playlists && songs->getCountProperty() == 3 &&
                   albums->getCountProperty() == 2 && artists->getCountProperty() == 2 &&
                   genres->getCountProperty() == 1 && playlists->getCountProperty() == 1,
                   "three tagged songs form two albums, two artists, one genre and one playlist")) return 1;
        Song* dawn = (*songs)[0];
        Song* dusk = (*songs)[1];
        Song* echo = (*songs)[2];
        Album* firstAlbum = (*albums)[0];
        Album* secondAlbum = (*albums)[1];
        Artist* ensemble = (*artists)[0];
        Artist* secondEnsemble = (*artists)[1];
        Genre* instrumental = (*genres)[0];
        Playlist* selected = (*playlists)[0];
        if (!check(dawn && dusk && echo && firstAlbum && secondAlbum && ensemble && secondEnsemble &&
                   instrumental && selected, "all indexed collection entries are library-owned objects")) return 1;
        check(dawn->getNameProperty() == "Dawn" && dusk->getNameProperty() == "Dusk" && echo->getNameProperty() == "Echo",
              "Song names come from tags rather than fixture basenames");
        check(dawn->getTrackNumberProperty() == 1 && dusk->getTrackNumberProperty() == 2 && echo->getTrackNumberProperty() == 1,
              "each Song retains its authored track number");
        check(ensemble->getNameProperty() == "Demo Ensemble" && secondEnsemble->getNameProperty() == "Second Ensemble" &&
              instrumental->getNameProperty() == "Instrumental", "artist and genre names come from authored tags");
        check(firstAlbum != secondAlbum && !firstAlbum->Equals(secondAlbum) && firstAlbum->getNameProperty() == "Field Notes" &&
              secondAlbum->getNameProperty() == "Field Notes" && firstAlbum->getArtistProperty() == ensemble &&
              secondAlbum->getArtistProperty() == secondEnsemble,
              "equal album titles by different artists remain distinct albums");
        const auto* firstSongs = firstAlbum->getSongsProperty();
        const auto* secondSongs = secondAlbum->getSongsProperty();
        if (!check(firstSongs && secondSongs && firstSongs->getCountProperty() == 2 && secondSongs->getCountProperty() == 1,
                   "album song collections group tracks by artist and album title")) return 1;
        check((*firstSongs)[0] == dawn && (*firstSongs)[1] == dusk && (*secondSongs)[0] == echo,
              "album members are the same Song objects exposed by the library");
        const auto* artistSongs = ensemble->getSongsProperty();
        const auto* otherSongs = secondEnsemble->getSongsProperty();
        check(artistSongs && otherSongs && artistSongs->getCountProperty() == 2 && otherSongs->getCountProperty() == 1 &&
              (*artistSongs)[0] == dawn && (*artistSongs)[1] == dusk && (*otherSongs)[0] == echo,
              "artist song views preserve library-owned member identity");
        const auto* artistAlbums = ensemble->getAlbumsProperty();
        const auto* otherAlbums = secondEnsemble->getAlbumsProperty();
        check(artistAlbums && otherAlbums && artistAlbums->getCountProperty() == 1 && otherAlbums->getCountProperty() == 1 &&
              (*artistAlbums)[0] == firstAlbum && (*otherAlbums)[0] == secondAlbum,
              "artist album views point to the corresponding library albums");
        const auto* genreSongs = instrumental->getSongsProperty();
        const auto* genreAlbums = instrumental->getAlbumsProperty();
        check(genreSongs && genreAlbums && genreSongs->getCountProperty() == 3 && genreAlbums->getCountProperty() == 2 &&
              (*genreSongs)[0] == dawn && (*genreSongs)[1] == dusk && (*genreSongs)[2] == echo &&
              (*genreAlbums)[0] == firstAlbum && (*genreAlbums)[1] == secondAlbum,
              "one genre shares all three songs and both artist-specific albums");
        check(firstAlbum->getGenreProperty() == instrumental && secondAlbum->getGenreProperty() == instrumental,
              "both albums refer to the same indexed genre");
        check(dawn->getAlbumProperty() == firstAlbum && dusk->getAlbumProperty() == firstAlbum &&
              echo->getAlbumProperty() == secondAlbum && dawn->getArtistProperty() == ensemble &&
              dusk->getArtistProperty() == ensemble && echo->getArtistProperty() == secondEnsemble &&
              dawn->getGenreProperty() == instrumental && dusk->getGenreProperty() == instrumental &&
              echo->getGenreProperty() == instrumental, "Song back-references agree with every group view");
        const auto* selectedSongs = selected->getSongsProperty();
        check(selected->getNameProperty() == "Selected" && selectedSongs && selectedSongs->getCountProperty() == 2 &&
              (*selectedSongs)[0] == dusk && (*selectedSongs)[1] == dawn,
              "M3U order reverses the first two library songs without duplicating their objects");
        std::cout << (passed ? "PASSED" : "FAILED") << " (" << checks << " checks)\n";
        // Library destruction releases all borrowed objects before cleanup removes the private files.
        return passed ? 0 : 1;
#endif
    }
}
int main()
{
    try { return MusicIndex(); }
    catch (const std::exception& error)
    {
        std::cerr << "Music library demo failed: " << error.what() << '\n';
        return 1;
    }
}

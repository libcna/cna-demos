// SPDX-License-Identifier: MS-PL
// Import tagged songs and follow library-owned album, artist, genre and playlist relationships.
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
#include "System/IO/MemoryStream.hpp"
#include "System/IO/Stream.hpp"
#include "System/ObjectDisposedException.hpp"

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

    template<typename Action>
    bool ThrowsObjectDisposed(Action&& action)
    {
        try
        {
            action();
        }
        catch (const System::ObjectDisposedException&)
        {
            return true;
        }
        catch (...)
        {
        }
        return false;
    }
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
        // Duration comes from the optional FFmpeg metadata probe, without starting playback.
#ifdef CNA_FFMPEG_AVAILABLE
        const auto dawnDuration = dawn->getDurationProperty();
        const auto duskDuration = dusk->getDurationProperty();
        const auto echoDuration = echo->getDurationProperty();
        check(dawnDuration.getTotalMillisecondsProperty() == 250.0 &&
              duskDuration.getTotalMillisecondsProperty() == 500.0 &&
              echoDuration.getTotalMillisecondsProperty() == 375.0,
              "Song durations match the three authored positive FLAC sample counts");
        check(firstAlbum->getDurationProperty() == dawnDuration + duskDuration &&
              secondAlbum->getDurationProperty() == echoDuration,
              "each album duration sums only its own artist-specific member songs");
        check(selected->getDurationProperty() == duskDuration + dawnDuration,
              "playlist duration sums its selected members and excludes the third song");
#else
        std::cout << "Duration aggregation not exercised: CNA was built without FFmpeg metadata probing\n";
#endif

        // A separately indexed library supplies distinct objects for real value comparisons.
        MediaLibrary mirror;
        auto* mirrorAlbums = mirror.getAlbumsProperty();
        auto* mirrorArtists = mirror.getArtistsProperty();
        auto* mirrorGenres = mirror.getGenresProperty();
        auto* mirrorPlaylists = mirror.getPlaylistsProperty();
        if (!check(mirrorAlbums && mirrorArtists && mirrorGenres && mirrorPlaylists &&
                       mirrorAlbums->getCountProperty() == 2 && mirrorArtists->getCountProperty() == 2 &&
                       mirrorGenres->getCountProperty() == 1 && mirrorPlaylists->getCountProperty() == 1,
                   "a second library independently indexes the same value records")) return 1;
        Album* mirrorFirstAlbum = (*mirrorAlbums)[0];
        Album* mirrorSecondAlbum = (*mirrorAlbums)[1];
        Artist* mirrorEnsemble = (*mirrorArtists)[0];
        Artist* mirrorSecondEnsemble = (*mirrorArtists)[1];
        Genre* mirrorInstrumental = (*mirrorGenres)[0];
        Playlist* mirrorSelected = (*mirrorPlaylists)[0];
        check(firstAlbum->Equals(mirrorFirstAlbum) &&
                  firstAlbum->Equals(static_cast<const System::Object*>(mirrorFirstAlbum)) &&
                  *firstAlbum == *mirrorFirstAlbum && *firstAlbum != *mirrorSecondAlbum &&
                  firstAlbum->GetHashCode() == mirrorFirstAlbum->GetHashCode() &&
                  firstAlbum->ToString() == "Field Notes",
              "album identity combines the title and artist across independent libraries");
        check(ensemble->Equals(mirrorEnsemble) &&
                  ensemble->Equals(static_cast<const System::Object*>(mirrorEnsemble)) &&
                  *ensemble == *mirrorEnsemble && *ensemble != *mirrorSecondEnsemble &&
                  ensemble->GetHashCode() == mirrorEnsemble->GetHashCode() &&
                  ensemble->ToString() == "Demo Ensemble",
              "artist identity, hash and text use the authored name across libraries");
        check(instrumental->Equals(mirrorInstrumental) &&
                  instrumental->Equals(static_cast<const System::Object*>(mirrorInstrumental)) &&
                  *instrumental == *mirrorInstrumental && !(*instrumental != *mirrorInstrumental) &&
                  instrumental->GetHashCode() == mirrorInstrumental->GetHashCode() &&
                  instrumental->ToString() == "Instrumental" &&
                  !instrumental->Equals(static_cast<const System::Object*>(mirrorSelected)),
              "genre identity, hash and text use its name and reject another media type");
        check(selected->Equals(mirrorSelected) &&
                  selected->Equals(static_cast<const System::Object*>(mirrorSelected)) &&
                  *selected == *mirrorSelected && !(*selected != *mirrorSelected) &&
                  selected->GetHashCode() == mirrorSelected->GetHashCode() &&
                  selected->ToString() == "Selected" &&
                  !selected->Equals(static_cast<const System::Object*>(mirrorInstrumental)),
              "playlist identity, hash and text use its name and reject another media type");

        check(!firstAlbum->getIsDisposedProperty() && !ensemble->getIsDisposedProperty() &&
                  !instrumental->getIsDisposedProperty() && !selected->getIsDisposedProperty(),
              "library-owned album, artist, genre and playlist views start live");
        firstAlbum->Dispose();
        ensemble->Dispose();
        instrumental->Dispose();
        selected->Dispose();
        firstAlbum->Dispose();
        ensemble->Dispose();
        instrumental->Dispose();
        selected->Dispose();
        check(firstAlbum->getIsDisposedProperty() && ensemble->getIsDisposedProperty() &&
                  instrumental->getIsDisposedProperty() && selected->getIsDisposedProperty() &&
                  !secondAlbum->getIsDisposedProperty() && !secondEnsemble->getIsDisposedProperty(),
              "object disposal is idempotent and marks only the selected borrowed views");

        int albumDisposalRefusals = 0;
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->getArtistProperty(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->getDurationProperty(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->getGenreProperty(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->getHasArtProperty(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->getNameProperty(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->getSongsProperty(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&]
        {
            std::unique_ptr<System::IO::Stream> disposedArt(firstAlbum->GetAlbumArt());
        });
        albumDisposalRefusals += ThrowsObjectDisposed([&]
        {
            std::unique_ptr<System::IO::Stream> disposedThumbnail(firstAlbum->GetThumbnail());
        });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->Equals(mirrorFirstAlbum); });
        albumDisposalRefusals += ThrowsObjectDisposed([&]
        {
            (void)firstAlbum->Equals(static_cast<const System::Object*>(mirrorFirstAlbum));
        });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*firstAlbum == *mirrorFirstAlbum); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->GetHashCode(); });
        albumDisposalRefusals += ThrowsObjectDisposed([&] { (void)firstAlbum->ToString(); });
        check(albumDisposalRefusals == 13,
              "a disposed Album refuses properties, image streams and value operations (CNA-DEMOS-ISSUE-098)");

        int artistDisposalRefusals = 0;
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)ensemble->getAlbumsProperty(); });
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)ensemble->getNameProperty(); });
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)ensemble->getSongsProperty(); });
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)ensemble->Equals(mirrorEnsemble); });
        artistDisposalRefusals += ThrowsObjectDisposed([&]
        {
            (void)ensemble->Equals(static_cast<const System::Object*>(mirrorEnsemble));
        });
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*ensemble == *mirrorEnsemble); });
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)ensemble->GetHashCode(); });
        artistDisposalRefusals += ThrowsObjectDisposed([&] { (void)ensemble->ToString(); });
        check(artistDisposalRefusals == 8,
              "a disposed Artist refuses properties and value operations (CNA-DEMOS-ISSUE-098)");

        int genreDisposalRefusals = 0;
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)instrumental->getAlbumsProperty(); });
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)instrumental->getNameProperty(); });
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)instrumental->getSongsProperty(); });
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)instrumental->Equals(mirrorInstrumental); });
        genreDisposalRefusals += ThrowsObjectDisposed([&]
        {
            (void)instrumental->Equals(static_cast<const System::Object*>(mirrorInstrumental));
        });
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*instrumental == *mirrorInstrumental); });
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)instrumental->GetHashCode(); });
        genreDisposalRefusals += ThrowsObjectDisposed([&] { (void)instrumental->ToString(); });
        check(genreDisposalRefusals == 8,
              "a disposed Genre refuses properties and value operations (CNA-DEMOS-ISSUE-098)");

        int playlistDisposalRefusals = 0;
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)selected->getDurationProperty(); });
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)selected->getNameProperty(); });
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)selected->getSongsProperty(); });
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)selected->Equals(mirrorSelected); });
        playlistDisposalRefusals += ThrowsObjectDisposed([&]
        {
            (void)selected->Equals(static_cast<const System::Object*>(mirrorSelected));
        });
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*selected == *mirrorSelected); });
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)selected->GetHashCode(); });
        playlistDisposalRefusals += ThrowsObjectDisposed([&] { (void)selected->ToString(); });
        check(playlistDisposalRefusals == 8,
              "a disposed Playlist refuses properties and value operations (CNA-DEMOS-ISSUE-098)");
        std::cout << "Disposed refusals: Album " << albumDisposalRefusals
                  << "/13, Artist " << artistDisposalRefusals << "/8, Genre "
                  << genreDisposalRefusals << "/8, Playlist " << playlistDisposalRefusals << "/8\n";

        check(!songs->getIsDisposedProperty() && !albums->getIsDisposedProperty() &&
                  !artists->getIsDisposedProperty() && !genres->getIsDisposedProperty() &&
                  !playlists->getIsDisposedProperty(),
              "the five top-level music collections remain live after individual view disposal");
        songs->Dispose();
        albums->Dispose();
        artists->Dispose();
        genres->Dispose();
        playlists->Dispose();
        songs->Dispose();
        albums->Dispose();
        artists->Dispose();
        genres->Dispose();
        playlists->Dispose();
        check(songs->getIsDisposedProperty() && albums->getIsDisposedProperty() &&
                  artists->getIsDisposedProperty() && genres->getIsDisposedProperty() &&
                  playlists->getIsDisposedProperty() && secondAlbum->getNameProperty() == "Field Notes",
              "collection disposal is idempotent and does not destroy its borrowed objects");

        int songCollectionDisposalRefusals = 0;
        songCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)songs->getCountProperty(); });
        songCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*songs)[0]; });
        check(songCollectionDisposalRefusals == 2,
              "a disposed SongCollection refuses Count and Item (CNA-DEMOS-ISSUE-098)");

        int albumCollectionDisposalRefusals = 0;
        albumCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)albums->getCountProperty(); });
        albumCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*albums)[0]; });
        check(albumCollectionDisposalRefusals == 2,
              "a disposed AlbumCollection refuses Count and Item (CNA-DEMOS-ISSUE-098)");

        int artistCollectionDisposalRefusals = 0;
        artistCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)artists->getCountProperty(); });
        artistCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*artists)[0]; });
        check(artistCollectionDisposalRefusals == 2,
              "a disposed ArtistCollection refuses Count and Item (CNA-DEMOS-ISSUE-098)");

        int genreCollectionDisposalRefusals = 0;
        genreCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)genres->getCountProperty(); });
        genreCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*genres)[0]; });
        check(genreCollectionDisposalRefusals == 2,
              "a disposed GenreCollection refuses Count and Item (CNA-DEMOS-ISSUE-098)");

        int playlistCollectionDisposalRefusals = 0;
        playlistCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)playlists->getCountProperty(); });
        playlistCollectionDisposalRefusals += ThrowsObjectDisposed([&] { (void)(*playlists)[0]; });
        check(playlistCollectionDisposalRefusals == 2,
              "a disposed PlaylistCollection refuses Count and Item (CNA-DEMOS-ISSUE-098)");
        std::cout << "Disposed collection refusals: Song " << songCollectionDisposalRefusals
                  << "/2, Album " << albumCollectionDisposalRefusals << "/2, Artist "
                  << artistCollectionDisposalRefusals << "/2, Genre " << genreCollectionDisposalRefusals
                  << "/2, Playlist " << playlistCollectionDisposalRefusals << "/2\n";

        check(!mirror.getIsDisposedProperty(), "the independent mirror library starts live");
        mirror.Dispose();
        mirror.Dispose();
        check(mirror.getIsDisposedProperty() && !mirrorAlbums->getIsDisposedProperty() &&
                  mirrorAlbums->getCountProperty() == 2 && !mirrorFirstAlbum->getIsDisposedProperty(),
              "library disposal is idempotent and does not cascade into retained collection or object views");

        const std::vector<std::uint8_t> disposedBytes{0x42};
        System::IO::MemoryStream disposedStream(disposedBytes.data(),
                                                static_cast<SharpRuntime::intcs>(disposedBytes.size()));
        int libraryDisposalRefusals = 0;
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getAlbumsProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getArtistsProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getGenresProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getMediaSourceProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getPicturesProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getPlaylistsProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getRootPictureAlbumProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getSavedPicturesProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.getSongsProperty(); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.GetPictureFromToken(""); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.SavePicture("DisposedBytes", disposedBytes); });
        libraryDisposalRefusals += ThrowsObjectDisposed([&] { (void)mirror.SavePicture("DisposedStream", &disposedStream); });
        check(libraryDisposalRefusals == 12,
              "a disposed MediaLibrary refuses all properties and picture methods (CNA-DEMOS-ISSUE-098)");
        std::cout << "Disposed MediaLibrary refusals: " << libraryDisposalRefusals << "/12\n";
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

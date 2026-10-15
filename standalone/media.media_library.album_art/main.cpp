// SPDX-License-Identifier: MS-PL
// Album cover and thumbnail streams in a private tagged music library.
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
#include <utility>

#include <iostream>
#include <string_view>
#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaLibrary.hpp"
#include "Microsoft/Xna/Framework/Media/Picture.hpp"
#include "Microsoft/Xna/Framework/Media/Album.hpp"
#include "Microsoft/Xna/Framework/Media/AlbumCollection.hpp"
#include "Microsoft/Xna/Framework/Media/Artist.hpp"
#include "System/InvalidOperationException.hpp"
#include "Microsoft/Xna/Framework/Media/PictureAlbumCollection.hpp"
#include "Microsoft/Xna/Framework/Media/PictureCollection.hpp"
#include "System/IO/MemoryStream.hpp"

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

    std::vector<std::uint8_t> ReadBytes(System::IO::Stream& stream)
    {
        const auto length = stream.getLengthProperty();
        if (length <= 0 || length > 16 * 1024 * 1024)
            throw std::runtime_error("Unexpected fixture stream length");
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
        SharpRuntime::intcs offset = 0;
        while (offset < length)
        {
            const auto count = stream.Read(bytes.data(), offset, length - offset);
            if (count <= 0) throw std::runtime_error("Fixture stream ended before its declared length");
            offset += count;
        }
        return bytes;
    }
#endif

    int AlbumArt()
    {
#if !defined(__linux__)
        std::cout << "SKIPPED: requires Linux XDG user-folder resolution\n";
        return 77;
#else
        // This standalone program configures its environment BEFORE its first CNA/SDL use.
        // SDL caches environment variables; changing native setenv inside an initialized browser
        // would not reliably update SDL's copy. No HOME changes or internal CNA path overrides.
        bool passed = true;
        int checks = 0;
        const auto check = [&](bool condition, std::string_view message) {
            ++checks;
            passed = passed && condition;
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            return condition;
        };
        const auto root = std::filesystem::temp_directory_path() /
            ("cna-demos-album-art-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(root)) throw std::runtime_error("Could not create private media directory");
        PrivateLibrary cleanup{root};
        const auto pictures = root / "pictures", music = root / "music", config = root / "config";
        std::filesystem::create_directory(pictures);
        std::filesystem::create_directory(music);
        std::filesystem::create_directory(config);
        if (root.string().find_first_of("\"\\\n") != std::string::npos)
            throw std::runtime_error("Temporary path cannot be represented in this XDG fixture");
        {
            std::ofstream settings(config / "user-dirs.dirs");
            settings << "XDG_PICTURES_DIR=\"" << pictures.string() << "\"\n"
                     << "XDG_MUSIC_DIR=\"" << music.string() << "\"\n";
            settings.close();
            if (!settings) throw std::runtime_error("Could not write the private XDG configuration");
        }
        if (::setenv("XDG_CONFIG_HOME", config.c_str(), 1) != 0)
            throw std::runtime_error("Could not select the private XDG configuration");
        auto* filesystem = CNA::Platform::GetCurrentPlatform().GetFileSystem();
        std::error_code error;
        // Resolve BOTH folders before constructing MediaLibrary or calling SavePicture.
        const bool pictureIsPrivate = filesystem && std::filesystem::equivalent(
            filesystem->GetUserFolder(CNA::Platform::UserFolder::Pictures), pictures, error);
        error.clear();
        const bool musicIsPrivate = filesystem && std::filesystem::equivalent(
            filesystem->GetUserFolder(CNA::Platform::UserFolder::Music), music, error);
        if (!pictureIsPrivate || !musicIsPrivate)
        {
            std::cout << "SKIPPED: platform does not honor the isolated Linux XDG media folders\n";
            return 77;
        }
        check(pictureIsPrivate && musicIsPrivate, "both media roots resolve to private fixture directories before library access");

        const auto covered = music / "covered", bare = music / "bare", embedded = music / "embedded";
        std::filesystem::create_directory(covered);
        std::filesystem::create_directory(bare);
        std::filesystem::create_directory(embedded);
        // The private folders hold one tagged album each; one gets a folder cover, one has no art,
        // and one carries an embedded FLAC PICTURE block.
        for (const auto& [name, folder] : std::vector<std::pair<std::string, std::filesystem::path>>{
                 {"track_a.flac", covered}, {"track_c.flac", bare},
                 {"track_embedded.flac", embedded}})
        {
            const auto input = TitleContainer::OpenStream("assets/media/library_music/" + name);
            const auto bytes = ReadBytes(*input);
            std::ofstream output(folder / name, std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            output.close();
            if (!output) throw std::runtime_error("Could not copy tagged music fixture");
        }
        const auto coverInput = TitleContainer::OpenStream("assets/media/library_panorama.png");
        const auto encoded = ReadBytes(*coverInput);
        const auto embeddedArtInput = TitleContainer::OpenStream("assets/textures/gradient.png");
        const auto embeddedArtBytes = ReadBytes(*embeddedArtInput);
        {
            std::ofstream output(covered / "cover.png", std::ios::binary);
            output.write(reinterpret_cast<const char*>(encoded.data()), static_cast<std::streamsize>(encoded.size()));
            output.close();
            if (!output) throw std::runtime_error("Could not copy original cover fixture");
        }
        // Unlike Album pointers, both returned streams are caller-owned.
        std::unique_ptr<System::IO::Stream> original, thumbnail;
        std::vector<std::uint8_t> thumbnailBytes;
        {
            MediaLibrary library;
            auto* albums = library.getAlbumsProperty();
            if (!check(albums && albums->getCountProperty() == 3,
                       "three private tagged folders create three albums")) return 1;
            Album* withArt = nullptr;
            Album* withoutArt = nullptr;
            Album* withEmbeddedArt = nullptr;
            for (int i = 0; i < albums->getCountProperty(); ++i)
            {
                auto* album = (*albums)[i];
                if (!album || !album->getArtistProperty()) continue;
                if (album->getArtistProperty()->getNameProperty() == "Demo Ensemble") withArt = album;
                if (album->getArtistProperty()->getNameProperty() == "Second Ensemble") withoutArt = album;
                if (album->getArtistProperty()->getNameProperty() == "Embedded Ensemble")
                    withEmbeddedArt = album;
            }
            if (!check(withArt && withoutArt && withEmbeddedArt && withArt->getHasArtProperty() &&
                           !withoutArt->getHasArtProperty() && withEmbeddedArt->getHasArtProperty(),
                       "HasArt distinguishes folder, embedded and missing album art")) return 1;
            bool missingOriginal = false, missingThumbnail = false;
            try { std::unique_ptr<System::IO::Stream> unused(withoutArt->GetAlbumArt()); }
            catch (const System::InvalidOperationException&) { missingOriginal = true; }
            try { std::unique_ptr<System::IO::Stream> unused(withoutArt->GetThumbnail()); }
            catch (const System::InvalidOperationException&) { missingThumbnail = true; }
            check(missingOriginal && missingThumbnail,
                  "both image requests reject the album without art with InvalidOperationException");
            original.reset(withArt->GetAlbumArt());
            thumbnail.reset(withArt->GetThumbnail());
            if (!check(original && thumbnail, "both album image requests return caller-owned streams")) return 1;
            check(ReadBytes(*original) == encoded, "GetAlbumArt returns the exact original cover bytes");
            thumbnailBytes = ReadBytes(*thumbnail);
            System::IO::MemoryStream previewInput(thumbnailBytes.data(),
                                                  static_cast<SharpRuntime::intcs>(thumbnailBytes.size()));
            Picture* preview = library.SavePicture("AlbumPreview.png", &previewInput);
            check(preview && preview->getWidthProperty() > 0 && preview->getWidthProperty() < 640 &&
                  preview->getWidthProperty() == 2 * preview->getHeightProperty() && thumbnailBytes != encoded,
                  "GetThumbnail produces a smaller decodable image with the original aspect ratio");
            if (preview) std::cout << "Album thumbnail: " << preview->getWidthProperty() << 'x'
                                   << preview->getHeightProperty() << '\n';

            std::unique_ptr<System::IO::Stream> embeddedOriginal(withEmbeddedArt->GetAlbumArt());
            std::unique_ptr<System::IO::Stream> firstEmbeddedThumbnail(withEmbeddedArt->GetThumbnail());
            std::unique_ptr<System::IO::Stream> secondEmbeddedThumbnail(withEmbeddedArt->GetThumbnail());
            const auto embeddedThumbnailBytes = ReadBytes(*firstEmbeddedThumbnail);
            Picture* embeddedPreview = library.SavePicture("EmbeddedAlbumPreview.png", embeddedThumbnailBytes);
            check(ReadBytes(*embeddedOriginal) == embeddedArtBytes &&
                      firstEmbeddedThumbnail != secondEmbeddedThumbnail &&
                      embeddedThumbnailBytes == embeddedArtBytes &&
                      ReadBytes(*secondEmbeddedThumbnail) == embeddedArtBytes && embeddedPreview &&
                      embeddedPreview->getWidthProperty() == 128 && embeddedPreview->getHeightProperty() == 128,
                  "embedded art and its independent thumbnail streams preserve the exact decodable PNG");
        }
        // Album and Picture pointers are now invalid; the independently owned streams survive.
        original->setPositionProperty(0);
        thumbnail->setPositionProperty(0);
        check(ReadBytes(*original) == encoded && ReadBytes(*thumbnail) == thumbnailBytes,
              "caller-owned cover and thumbnail streams remain readable after library destruction");

        const auto preferredInput = TitleContainer::OpenStream("assets/textures/checker.png");
        const auto preferredBytes = ReadBytes(*preferredInput);
        const auto lowerPriorityInput = TitleContainer::OpenStream("assets/textures/arrow.png");
        const auto lowerPriorityBytes = ReadBytes(*lowerPriorityInput);
        for (const auto& [path, bytes] :
             std::vector<std::pair<std::filesystem::path, const std::vector<std::uint8_t>*>>{
                 {bare / "COVER.PNG", &preferredBytes},
                 {bare / "FoLdEr.PnG", &lowerPriorityBytes},
                 {embedded / "cover.png", &preferredBytes}})
        {
            std::ofstream output(path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes->data()),
                         static_cast<std::streamsize>(bytes->size()));
            output.close();
            if (!output) throw std::runtime_error("Could not write album-art priority fixture");
        }

        const auto selectedBareArt = [&]() -> std::vector<std::uint8_t>
        {
            MediaLibrary scan;
            for (Album* candidate : *scan.getAlbumsProperty())
            {
                if (candidate && candidate->getArtistProperty() &&
                    candidate->getArtistProperty()->getNameProperty() == "Second Ensemble" &&
                    candidate->getHasArtProperty())
                {
                    std::unique_ptr<System::IO::Stream> art(candidate->GetAlbumArt());
                    return ReadBytes(*art);
                }
            }
            return {};
        };

        MediaLibrary priorityLibrary;
        Album* priorityAlbum = nullptr;
        Album* folderOverrideAlbum = nullptr;
        for (Album* candidate : *priorityLibrary.getAlbumsProperty())
        {
            if (candidate && candidate->getArtistProperty() &&
                candidate->getArtistProperty()->getNameProperty() == "Second Ensemble")
            {
                priorityAlbum = candidate;
            }
            if (candidate && candidate->getArtistProperty() &&
                candidate->getArtistProperty()->getNameProperty() == "Embedded Ensemble")
            {
                folderOverrideAlbum = candidate;
            }
        }
        if (!check(priorityAlbum && priorityAlbum->getHasArtProperty(),
                   "case-insensitive album-art candidates make the formerly bare album report art")) return 1;
        std::unique_ptr<System::IO::Stream> preferredArt(priorityAlbum->GetAlbumArt());
        check(ReadBytes(*preferredArt) == preferredBytes,
              "cover names outrank folder names independently of filename case");
        std::unique_ptr<System::IO::Stream> folderOverrideArt(
            folderOverrideAlbum ? folderOverrideAlbum->GetAlbumArt() : nullptr);
        check(folderOverrideAlbum && folderOverrideArt && ReadBytes(*folderOverrideArt) == preferredBytes,
              "folder cover art takes precedence over a member track's embedded image");
        std::unique_ptr<System::IO::Stream> firstSmallThumbnail(priorityAlbum->GetThumbnail());
        std::unique_ptr<System::IO::Stream> secondSmallThumbnail(priorityAlbum->GetThumbnail());
        const auto firstSmallBytes = ReadBytes(*firstSmallThumbnail);
        const auto secondSmallBytes = ReadBytes(*secondSmallThumbnail);
        Picture* smallPreview = priorityLibrary.SavePicture("SmallAlbumPreview.png", firstSmallBytes);
        check(firstSmallThumbnail != secondSmallThumbnail && firstSmallBytes == preferredBytes &&
                  secondSmallBytes == preferredBytes && smallPreview &&
                  smallPreview->getWidthProperty() == 64 && smallPreview->getHeightProperty() == 64,
              "an already-small album cover returns exact bytes through independent thumbnail streams");

        // One representative image per extension is enough to expose the documented ordering;
        // the encoded bytes deliberately stay self-describing so GetAlbumArt can be compared exactly.
        {
            std::ofstream jpeg(bare / "cover.jpeg", std::ios::binary);
            jpeg.write(reinterpret_cast<const char*>(embeddedArtBytes.data()),
                       static_cast<std::streamsize>(embeddedArtBytes.size()));
            std::ofstream jpg(bare / "cover.jpg", std::ios::binary);
            jpg.write(reinterpret_cast<const char*>(lowerPriorityBytes.data()),
                      static_cast<std::streamsize>(lowerPriorityBytes.size()));
            jpeg.close();
            jpg.close();
            if (!jpeg || !jpg) throw std::runtime_error("Could not write album-art extension fixtures");
        }
        check(selectedBareArt() == lowerPriorityBytes,
              "cover.jpg outranks cover.jpeg and cover.png within one filename family");
        error.clear();
        if (!std::filesystem::remove(bare / "cover.jpg", error) || error)
            throw std::runtime_error("Could not remove the highest-priority cover candidate");
        check(selectedBareArt() == embeddedArtBytes,
              "cover.jpeg is recognized and outranks the remaining cover.png candidate");
        error.clear();
        if (!std::filesystem::remove(bare / "cover.jpeg", error) || error)
            throw std::runtime_error("Could not remove the JPEG cover candidate");

        const std::vector<std::uint8_t> corruptBytes{
            'n', 'o', 't', ' ', 'a', 'n', ' ', 'i', 'm', 'a', 'g', 'e'};
        {
            std::ofstream output(bare / "COVER.PNG", std::ios::binary | std::ios::trunc);
            output.write(reinterpret_cast<const char*>(corruptBytes.data()),
                         static_cast<std::streamsize>(corruptBytes.size()));
            output.close();
            if (!output) throw std::runtime_error("Could not corrupt the selected album-art fixture");
        }
        std::unique_ptr<System::IO::Stream> corruptArt(priorityAlbum->GetAlbumArt());
        std::unique_ptr<System::IO::Stream> corruptThumbnail(priorityAlbum->GetThumbnail());
        check(priorityAlbum->getHasArtProperty() && ReadBytes(*corruptArt) == corruptBytes &&
                  ReadBytes(*corruptThumbnail) == corruptBytes,
              "a selected corrupt cover stays available and thumbnail generation falls back to its exact bytes");

        error.clear();
        if (!std::filesystem::remove(bare / "COVER.PNG", error) || error)
            throw std::runtime_error("Could not remove the cover-family fixture");
        error.clear();
        if (!std::filesystem::remove(bare / "FoLdEr.PnG", error) || error)
            throw std::runtime_error("Could not remove the folder-family fixture");
        for (const auto& [path, bytes] :
             std::vector<std::pair<std::filesystem::path, const std::vector<std::uint8_t>*>>{
                 {bare / "front.jpeg", &preferredBytes},
                 {bare / "album.jpg", &lowerPriorityBytes},
                 {bare / "albumart.png", &embeddedArtBytes}})
        {
            std::ofstream output(path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(bytes->data()),
                         static_cast<std::streamsize>(bytes->size()));
            output.close();
            if (!output) throw std::runtime_error("Could not write album-art family fixture");
        }
        check(selectedBareArt() == preferredBytes,
              "front art outranks the album and albumart filename families");
        error.clear();
        if (!std::filesystem::remove(bare / "front.jpeg", error) || error)
            throw std::runtime_error("Could not remove the front-family fixture");
        check(selectedBareArt() == lowerPriorityBytes,
              "album art becomes active before the lower albumart fallback");
        error.clear();
        if (!std::filesystem::remove(bare / "album.jpg", error) || error)
            throw std::runtime_error("Could not remove the album-family fixture");
        check(selectedBareArt() == embeddedArtBytes,
              "albumart.png is accepted as the final recognized filename fallback");
        std::cout << (passed ? "PASSED" : "FAILED") << " (" << checks << " checks)\n";
        return passed ? 0 : 1;
#endif
    }
}

int main()
{
    try { return AlbumArt(); }
    catch (const std::exception& error)
    {
        std::cerr << "Album art demo failed: " << error.what() << '\n';
        return 1;
    }
}

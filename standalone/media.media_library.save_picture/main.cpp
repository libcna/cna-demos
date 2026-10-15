// SPDX-License-Identifier: MS-PL
// SavePicture and picture streams in a private media library, isolated in a child process.
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

#include <iostream>
#include <string_view>
#include "CNA/Platform/CurrentPlatform.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaLibrary.hpp"
#include "Microsoft/Xna/Framework/Media/Picture.hpp"
#include "Microsoft/Xna/Framework/Media/PictureAlbum.hpp"
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

    // Stream::Read may return fewer bytes than requested before EOF. The source stays a valid PNG.
    class ShortReadStream final : public System::IO::MemoryStream
    {
    public:
        using System::IO::MemoryStream::MemoryStream;
        SharpRuntime::intcs Read(SharpRuntime::bytecs buffer[], SharpRuntime::intcs offset,
                                SharpRuntime::intcs count) override
        {
            ++calls;
            return System::IO::MemoryStream::Read(buffer, offset, std::min<SharpRuntime::intcs>(count, 13));
        }
        int calls = 0;
    };

    std::vector<std::uint8_t> ReadImage(System::IO::Stream& stream)
    {
        const auto length = stream.getLengthProperty();
        if (length <= 0 || length > 16 * 1024 * 1024)
            throw std::runtime_error("Unexpected image stream length");
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
        SharpRuntime::intcs offset = 0;
        while (offset < length)
        {
            const auto count = stream.Read(bytes.data(), offset, length - offset);
            if (count <= 0) throw std::runtime_error("Image stream ended before its declared length");
            offset += count;
        }
        return bytes;
    }

    void WriteImage(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
    {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
        file.close();
        if (!file) throw std::runtime_error("Could not write the nested picture fixture");
    }
#endif

    int SavePicture()
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
            ("cna-demos-pictures-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
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

        MediaLibrary library;
        check(library.getPicturesProperty()->getCountProperty() == 0 &&
                  library.getSavedPicturesProperty()->getCountProperty() == 0,
                  "the private library starts with no pictures");
        const auto input = TitleContainer::OpenStream("assets/media/library_panorama.png");
        const auto encoded = ReadImage(*input);
        const auto saveStarted = std::chrono::system_clock::now();
        Picture* picture = library.SavePicture("Panorama.png", encoded); // borrowed: library owns it
        const auto saveFinished = std::chrono::system_clock::now();
        if (!check(picture != nullptr, "SavePicture returns the new library-owned picture")) return 1;
        check(picture->getNameProperty() == "Panorama.png" && picture->getWidthProperty() == 640 &&
                  picture->getHeightProperty() == 320, "the saved picture retains its name and decoded dimensions");
        check(picture->getDateProperty() >= saveStarted && picture->getDateProperty() <= saveFinished,
              "the saved picture date records this save operation");
        auto* album = picture->getAlbumProperty();
        if (!check(album && album->getNameProperty() == "Saved Pictures" &&
                        album->getParentProperty() == library.getRootPictureAlbumProperty(),
                        "the new picture belongs to Saved Pictures below the root album")) return 1;
        check(library.getPicturesProperty()->getCountProperty() == 1 &&
                  library.getSavedPicturesProperty()->getCountProperty() == 1 &&
                  album->getPicturesProperty()->getCountProperty() == 1 &&
                  library.getRootPictureAlbumProperty()->getAlbumsProperty()->getCountProperty() == 1,
                  "all library and album views include the newly saved picture");
        // Returned streams are caller-owned, unlike Picture and PictureAlbum pointers.
        std::unique_ptr<System::IO::Stream> original(picture->GetImage());
        check(ReadImage(*original) == encoded, "GetImage returns the exact original encoded bytes");
        std::unique_ptr<System::IO::Stream> thumbnail(picture->GetThumbnail());
        const auto thumbnailBytes = ReadImage(*thumbnail);
        System::IO::MemoryStream thumbnailInput(thumbnailBytes.data(),
                                                static_cast<SharpRuntime::intcs>(thumbnailBytes.size()));
        Picture* preview = library.SavePicture("Preview.png", &thumbnailInput);
        if (!check(preview != nullptr, "the stream overload saves the generated thumbnail")) return 1;
        check(preview->getWidthProperty() > 0 && preview->getWidthProperty() < picture->getWidthProperty() &&
                  preview->getWidthProperty() == 2 * preview->getHeightProperty(),
                  "the thumbnail is smaller and preserves the panorama's aspect ratio");
        check(thumbnailInput.getPositionProperty() == thumbnailInput.getLengthProperty(),
                  "SavePicture consumes the supplied memory stream");
        std::cout << "Original " << picture->getWidthProperty() << 'x' << picture->getHeightProperty()
                  << ", thumbnail " << preview->getWidthProperty() << 'x' << preview->getHeightProperty() << '\n';
        MediaLibrary reopened;
        check(reopened.getPicturesProperty()->getCountProperty() == 2 &&
                  reopened.getSavedPicturesProperty()->getCountProperty() == 2,
                  "a new library indexes both files from the private saved-pictures folder");
        Picture* firstIndexed = (*reopened.getPicturesProperty())[0];
        Picture* secondIndexed = (*reopened.getPicturesProperty())[1];
        auto* rootAlbums = reopened.getRootPictureAlbumProperty()->getAlbumsProperty();
        if (!check(rootAlbums && rootAlbums->getCountProperty() == 1,
                   "the reopened root exposes the one saved-pictures album")) return 1;
        PictureAlbum* reopenedSavedAlbum = (*rootAlbums)[0];
        check(firstIndexed && secondIndexed && firstIndexed != secondIndexed &&
                  ((firstIndexed->getNameProperty() == "Panorama.png" &&
                    secondIndexed->getNameProperty() == "Preview.png") ||
                   (firstIndexed->getNameProperty() == "Preview.png" &&
                    secondIndexed->getNameProperty() == "Panorama.png")) &&
                  reopenedSavedAlbum == firstIndexed->getAlbumProperty() &&
                  reopenedSavedAlbum == secondIndexed->getAlbumProperty(),
              "picture and album collection indexers expose both saved files under one album");
        Picture* indexedPicture = reopened.GetPictureFromToken(picture->getTokenEXT());
        check(indexedPicture != nullptr &&
                  reopened.GetPictureFromToken(picture->getTokenEXT() + ".missing") == nullptr &&
                  indexedPicture->getDateProperty().time_since_epoch() !=
                      std::chrono::system_clock::duration::zero(),
              "the picture token resolves across libraries and the indexed date is populated");
        check(picture->Equals(indexedPicture) &&
                  picture->Equals(static_cast<const System::Object*>(indexedPicture)) &&
                  *picture == *indexedPicture && *picture != *preview &&
                  picture->GetHashCode() == indexedPicture->GetHashCode() &&
                  picture->ToString() == "Panorama.png" &&
                  !picture->Equals(static_cast<const System::Object*>(reopenedSavedAlbum)),
              "picture identity uses its saved path while diagnostic text uses its name");
        check(album->Equals(reopenedSavedAlbum) &&
                  album->Equals(static_cast<const System::Object*>(reopenedSavedAlbum)) &&
                  *album == *reopenedSavedAlbum && !(*album != *reopenedSavedAlbum) &&
                  album->GetHashCode() == reopenedSavedAlbum->GetHashCode() &&
                  album->ToString() == "Saved Pictures" &&
                  !album->Equals(static_cast<const System::Object*>(indexedPicture)),
              "picture-album identity uses its path and rejects another media type");
        ShortReadStream control(encoded.data(), static_cast<SharpRuntime::intcs>(encoded.size()));
        check(ReadImage(control) == encoded && control.calls > 1,
              "the short-read fixture delivers the complete valid PNG across multiple reads");
        ShortReadStream chunks(encoded.data(), static_cast<SharpRuntime::intcs>(encoded.size()));
        Picture* chunked = library.SavePicture("Chunked.png", &chunks);
        if (!check(chunked != nullptr, "the short-read save returns a library-owned picture")) return 1;
        std::unique_ptr<System::IO::Stream> savedChunks(chunked->GetImage());
        const auto savedBytes = ReadImage(*savedChunks);
        std::cout << "Short reads: " << chunks.calls << " call(s), consumed " << chunks.getPositionProperty()
                  << '/' << chunks.getLengthProperty() << " bytes; saved picture " << chunked->getWidthProperty()
                  << 'x' << chunked->getHeightProperty() << '\n';
        check(savedBytes == encoded && chunks.getPositionProperty() == chunks.getLengthProperty(),
              "SavePicture consumes every chunk and preserves the complete encoded image (CNA-DEMOS-ISSUE-041)");

        // Add an ordinary two-level folder tree outside Saved Pictures and let a fresh public
        // MediaLibrary discover it. This keeps the test about album relationships rather than
        // using an internal index or constructing PictureAlbum values directly.
        const auto tripsPath = pictures / "Trips";
        const auto dayPath = tripsPath / "Day One";
        std::filesystem::create_directories(dayPath);
        WriteImage(tripsPath / "overview.png", encoded);
        WriteImage(dayPath / "frame.png", encoded);
        MediaLibrary hierarchy;
        PictureAlbum* hierarchyRoot = hierarchy.getRootPictureAlbumProperty();
        PictureAlbum* tripsAlbum = nullptr;
        if (hierarchyRoot && hierarchyRoot->getAlbumsProperty())
        {
            for (PictureAlbum* candidate : *hierarchyRoot->getAlbumsProperty())
            {
                if (candidate && candidate->getNameProperty() == "Trips") tripsAlbum = candidate;
            }
        }
        PictureAlbum* dayAlbum = nullptr;
        if (tripsAlbum && tripsAlbum->getAlbumsProperty())
        {
            for (PictureAlbum* candidate : *tripsAlbum->getAlbumsProperty())
            {
                if (candidate && candidate->getNameProperty() == "Day One") dayAlbum = candidate;
            }
        }
        check(tripsAlbum && dayAlbum && tripsAlbum->getParentProperty() == hierarchyRoot &&
                  dayAlbum->getParentProperty() == tripsAlbum,
              "a fresh library preserves both levels and parent links of an ordinary album tree");
        Picture* overview = tripsAlbum && tripsAlbum->getPicturesProperty() &&
                                    tripsAlbum->getPicturesProperty()->getCountProperty() == 1
                                ? (*tripsAlbum->getPicturesProperty())[0]
                                : nullptr;
        Picture* frame = dayAlbum && dayAlbum->getPicturesProperty() &&
                                 dayAlbum->getPicturesProperty()->getCountProperty() == 1
                             ? (*dayAlbum->getPicturesProperty())[0]
                             : nullptr;
        check(overview && frame && overview->getNameProperty() == "overview" &&
                  frame->getNameProperty() == "frame" && overview->getAlbumProperty() == tripsAlbum &&
                  frame->getAlbumProperty() == dayAlbum && overview->getWidthProperty() == 640 &&
                  frame->getHeightProperty() == 320,
              "each nested album exposes only its own decoded picture with the matching back-reference");
        if (overview && frame)
        {
            std::unique_ptr<System::IO::Stream> overviewImage(overview->GetImage());
            std::unique_ptr<System::IO::Stream> frameImage(frame->GetImage());
            check(ReadImage(*overviewImage) == encoded && ReadImage(*frameImage) == encoded,
                  "nested picture streams retain the exact original encoded bytes");
        }
        else
        {
            check(false, "nested picture streams retain the exact original encoded bytes");
        }

        const auto savedPicturesPath = pictures / "Saved Pictures";
        const auto regularFileCount = [](const std::filesystem::path& path)
        {
            return std::count_if(std::filesystem::directory_iterator(path),
                                 std::filesystem::directory_iterator(), [](const auto& entry)
                                 { return entry.is_regular_file(); });
        };
        const auto savedFilesBeforeCollision = regularFileCount(savedPicturesPath);
        Picture* replacement = library.SavePicture("Panorama.png", thumbnailBytes);
        check(replacement && replacement != picture &&
                  regularFileCount(savedPicturesPath) == savedFilesBeforeCollision &&
                  library.getSavedPicturesProperty()->getCountProperty() == 4 &&
                  album->getPicturesProperty()->getCountProperty() == 4,
              "saving the same name replaces one file but appends a distinct live picture view");
        check(replacement && replacement->Equals(picture) &&
                  replacement->getTokenEXT() == picture->getTokenEXT() &&
                  replacement->getWidthProperty() == 128 && replacement->getHeightProperty() == 64 &&
                  picture->getWidthProperty() == 640 && picture->getHeightProperty() == 320,
              "same-path picture views share identity while retaining their creation-time dimensions");
        MediaLibrary collisionIndex;
        Picture* indexedReplacement = collisionIndex.GetPictureFromToken(picture->getTokenEXT());
        if (indexedReplacement)
        {
            std::unique_ptr<System::IO::Stream> replacementImage(indexedReplacement->GetImage());
            check(indexedReplacement->getWidthProperty() == 128 &&
                      indexedReplacement->getHeightProperty() == 64 &&
                      ReadImage(*replacementImage) == thumbnailBytes,
                  "a fresh library indexes the replacement dimensions and exact latest bytes");
        }
        else
        {
            check(false, "a fresh library indexes the replacement dimensions and exact latest bytes");
        }

        check(!picture->getIsDisposedProperty() && !album->getIsDisposedProperty(),
              "library-owned picture and picture-album views start live");
        picture->Dispose();
        album->Dispose();
        picture->Dispose();
        album->Dispose();
        check(picture->getIsDisposedProperty() && album->getIsDisposedProperty() &&
                  !preview->getIsDisposedProperty() && !chunked->getIsDisposedProperty() &&
                  !replacement->getIsDisposedProperty() && picture->getAlbumProperty() == album &&
                  album->getPicturesProperty()->getCountProperty() == 4,
              "object disposal is idempotent, marks only that borrowed view and preserves its relationships");

        auto* reopenedPictures = reopened.getPicturesProperty();
        check(!reopenedPictures->getIsDisposedProperty() && !rootAlbums->getIsDisposedProperty(),
              "reopened picture and album collection views start live");
        reopenedPictures->Dispose();
        rootAlbums->Dispose();
        reopenedPictures->Dispose();
        rootAlbums->Dispose();
        check(reopenedPictures->getIsDisposedProperty() && rootAlbums->getIsDisposedProperty() &&
                  reopenedPictures->getCountProperty() == 0 && rootAlbums->getCountProperty() == 0 &&
                  indexedPicture->getNameProperty() == "Panorama.png" &&
                  reopenedSavedAlbum->getNameProperty() == "Saved Pictures",
              "collection disposal is idempotent and clears each non-owning view without destroying its objects");
        std::cout << (passed ? "PASSED" : "FAILED") << " (" << checks << " checks)\n";
        return passed ? 0 : 1;
#endif
    }
}

int main()
{
    try { return SavePicture(); }
    catch (const std::exception& error)
    {
        std::cerr << "Picture library demo failed: " << error.what() << '\n';
        return 1;
    }
}

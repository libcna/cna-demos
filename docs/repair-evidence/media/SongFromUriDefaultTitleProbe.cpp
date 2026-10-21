// SPDX-License-Identifier: MS-PL
// ISSUE-015: resolve relative music before Game/TitleLocation initializes the bridge.
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "CNA/Internal/TitlePath.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include "System/Uri.hpp"

namespace fs = std::filesystem;
using Microsoft::Xna::Framework::Media::Song;
namespace
{
    struct OwnedRoot
    {
        fs::path path;
        explicit OwnedRoot(const fs::path& title)
            : path(title / ("song-uri-default-" +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())))
        {
            if (!fs::create_directory(path)) throw std::runtime_error("Fixture already exists");
        }
        ~OwnedRoot() { std::error_code error; fs::remove_all(path, error); }
    };
    struct RestoreWorking
    {
        fs::path previous = fs::current_path();
        ~RestoreWorking() { std::error_code error; fs::current_path(previous, error); }
    };
}
int main(int argc, char** argv)
{
    try
    {
        if (argc != 3) throw std::runtime_error("Pass string/typed and the absolute audio fixture");
        if (CNA::Internal::TryGetTitlePath()) throw std::runtime_error("Title bridge is not cold");
        const std::string mode = argv[1];
        if (mode != "string" && mode != "typed") throw std::runtime_error("Unknown overload");
        const fs::path title = fs::absolute(argv[0]).parent_path();
        const fs::path source = fs::absolute(argv[2]);
        OwnedRoot root(title);
        RestoreWorking restore;
        const fs::path file = root.path / "song.ogg", working = root.path / "cwd";
        fs::create_directory(working);
        fs::copy_file(source, file);
        const std::string relative = (root.path.filename() / "song.ogg").generic_string();
        // A real decoy makes the old cwd behavior observable even when it can open a file.
        fs::create_directory(working / root.path.filename());
        fs::copy_file(source, working / relative);
        fs::current_path(working);
        std::unique_ptr<Song> song(mode == "typed" ?
            Song::FromUri("Cold title", System::Uri(relative, System::UriKind::Relative)) :
            Song::FromUri("Cold title", relative));
        const bool correct = song && song->getNameProperty() == "Cold title" &&
            fs::equivalent(song->getHandle(), file) &&
            !fs::equivalent(song->getHandle(), working / relative);
        std::cout << "cold title " << mode << " resolved=" << correct << '\n';
        return correct ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 2;
    }
}

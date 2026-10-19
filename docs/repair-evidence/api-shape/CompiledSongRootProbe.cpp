// SPDX-License-Identifier: MS-PL
// ISSUE-020: the positive demo's narrower manager root must not conceal relocation failure.
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"
#include <exception>
#include <iostream>
#include <string>
int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    using Microsoft::Xna::Framework::Content::ContentManager;
    using Microsoft::Xna::Framework::Media::Song;
    try
    {
        ContentManager narrow(nullptr, std::string(argv[1]) + "/media");
        const Song control = narrow.Load<Song>("music/theme");
        if (control.getDurationProperty().getTicksProperty() <= 0) return 2;
        std::cout << "PASS original narrower-root load\n";
        ContentManager parent(nullptr, argv[1]);
        const Song relocated = parent.Load<Song>("media/music/theme");
        const bool matches = relocated.getDurationProperty() == control.getDurationProperty();
        std::cout << (matches ? "PASS" : "FAIL") << " same asset under parent manager root\n";
        return matches ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cout << "FAIL " << error.what() << '\n';
        return 1;
    }
}

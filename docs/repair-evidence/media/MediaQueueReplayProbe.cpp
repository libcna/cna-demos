// Source-only follow-up probe. Do not add to demo catalog before independent review.
#include <iostream>
#include <memory>
#include <string>
#include "Microsoft/Xna/Framework/Media/MediaPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/MediaQueue.hpp"
#include "Microsoft/Xna/Framework/Media/Song.hpp"

using namespace Microsoft::Xna::Framework::Media;
int main(int argc, char** argv)
{
    if (argc != 3) return 2;
    const bool alias = std::string(argv[1]) == "queue";
    const std::string name(128, 'Q');
    std::unique_ptr<Song> caller(new Song(argv[2], name));
    auto& queue = MediaPlayer::getQueueProperty();
    queue.Add(new Song(argv[2], name));
    queue.setActiveSongIndexProperty(0);
    Song* selected = alias ? queue.getActiveSongProperty() : caller.get();
    if (!selected || selected->getNameProperty() != name || queue.getCountProperty() != 1) return 3;
    std::cout << "Before Play: valid " << (alias ? "queue-owned" : "caller-owned")
              << " Song, name length " << selected->getNameProperty().size() << std::endl;
    // The pointer is valid at entry. Play clears its own queue internally before loading song.
    MediaPlayer::Play(selected);
    const bool copied = queue.getCountProperty() == 1 && queue.getActiveSongProperty() &&
                        queue.getActiveSongProperty()->getNameProperty() == name;
    MediaPlayer::Stop();
    queue.Clear();
    std::cout << "After Play: expected name copied = " << copied << std::endl;
    return copied ? 0 : 1;
}

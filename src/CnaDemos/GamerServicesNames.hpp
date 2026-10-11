// SPDX-License-Identifier: MS-PL
#pragma once
// Names of gamer services enumeration values, for demo output (Name() overloads, as in InputNames.hpp).
// Header-only: only the categories that link CNA_GamerServices include it.

#include <string_view>

#include "Microsoft/Xna/Framework/GamerServices/GameDifficulty.hpp"
#include "Microsoft/Xna/Framework/GamerServices/ControllerSensitivity.hpp"
#include "Microsoft/Xna/Framework/GamerServices/RacingCameraAngle.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerZone.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPrivilegeSetting.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPresenceMode.hpp"

namespace CnaDemos
{
    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::GamerServices::GameDifficulty value)
    {
        using enum Microsoft::Xna::Framework::GamerServices::GameDifficulty;
        switch (value)
        {
            case Easy: return "Easy";
            case Normal: return "Normal";
            case Hard: return "Hard";
            default: return "?";
        }
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::GamerServices::ControllerSensitivity value)
    {
        using enum Microsoft::Xna::Framework::GamerServices::ControllerSensitivity;
        switch (value)
        {
            case Low: return "Low";
            case Medium: return "Medium";
            case High: return "High";
            default: return "?";
        }
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::GamerServices::RacingCameraAngle value)
    {
        using enum Microsoft::Xna::Framework::GamerServices::RacingCameraAngle;
        switch (value)
        {
            case Back: return "Back";
            case Front: return "Front";
            case Inside: return "Inside";
            default: return "?";
        }
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::GamerServices::GamerZone value)
    {
        using enum Microsoft::Xna::Framework::GamerServices::GamerZone;
        switch (value)
        {
            case Unknown: return "Unknown";
            case Recreation: return "Recreation";
            case Pro: return "Pro";
            case Family: return "Family";
            case Underground: return "Underground";
            default: return "?";
        }
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::GamerServices::GamerPrivilegeSetting value)
    {
        using enum Microsoft::Xna::Framework::GamerServices::GamerPrivilegeSetting;
        switch (value)
        {
            case Blocked: return "Blocked";
            case FriendsOnly: return "FriendsOnly";
            case Everyone: return "Everyone";
            default: return "?";
        }
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::GamerServices::GamerPresenceMode value)
    {
        using enum Microsoft::Xna::Framework::GamerServices::GamerPresenceMode;
        switch (value)
        {
            case None: return "None";
            case SinglePlayer: return "SinglePlayer";
            case Multiplayer: return "Multiplayer";
            case LocalCoOp: return "LocalCoOp";
            case LocalVersus: return "LocalVersus";
            case OnlineCoOp: return "OnlineCoOp";
            case OnlineVersus: return "OnlineVersus";
            case VersusComputer: return "VersusComputer";
            case Stage: return "Stage";
            case Level: return "Level";
            case CoOpStage: return "CoOpStage";
            case CoOpLevel: return "CoOpLevel";
            case ArcadeMode: return "ArcadeMode";
            case CampaignMode: return "CampaignMode";
            case ChallengeMode: return "ChallengeMode";
            case ExplorationMode: return "ExplorationMode";
            case PracticeMode: return "PracticeMode";
            case PuzzleMode: return "PuzzleMode";
            case ScenarioMode: return "ScenarioMode";
            case StoryMode: return "StoryMode";
            case SurvivalMode: return "SurvivalMode";
            case TutorialMode: return "TutorialMode";
            case DifficultyEasy: return "DifficultyEasy";
            case DifficultyMedium: return "DifficultyMedium";
            case DifficultyHard: return "DifficultyHard";
            case DifficultyExtreme: return "DifficultyExtreme";
            case Score: return "Score";
            case VersusScore: return "VersusScore";
            case Winning: return "Winning";
            case Losing: return "Losing";
            case ScoreIsTied: return "ScoreIsTied";
            case Outnumbered: return "Outnumbered";
            case OnARoll: return "OnARoll";
            case InCombat: return "InCombat";
            case BattlingBoss: return "BattlingBoss";
            case TimeAttack: return "TimeAttack";
            case TryingForRecord: return "TryingForRecord";
            case FreePlay: return "FreePlay";
            case WastingTime: return "WastingTime";
            case StuckOnAHardBit: return "StuckOnAHardBit";
            case NearlyFinished: return "NearlyFinished";
            case LookingForGames: return "LookingForGames";
            case WaitingForPlayers: return "WaitingForPlayers";
            case WaitingInLobby: return "WaitingInLobby";
            case SettingUpMatch: return "SettingUpMatch";
            case PlayingWithFriends: return "PlayingWithFriends";
            case AtMenu: return "AtMenu";
            case StartingGame: return "StartingGame";
            case Paused: return "Paused";
            case GameOver: return "GameOver";
            case WonTheGame: return "WonTheGame";
            case ConfiguringSettings: return "ConfiguringSettings";
            case CustomizingPlayer: return "CustomizingPlayer";
            case EditingLevel: return "EditingLevel";
            case InGameStore: return "InGameStore";
            case WatchingCutscene: return "WatchingCutscene";
            case WatchingCredits: return "WatchingCredits";
            case PlayingMinigame: return "PlayingMinigame";
            case FoundSecret: return "FoundSecret";
            case CornflowerBlue: return "CornflowerBlue";
            default: return "?";
        }
    }
}

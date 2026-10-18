// Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

#include <iostream>
#include <string_view>
#include <filesystem>
#include <MeddySDK/Meddyproject/Utils.h>
#include <MeddySDK/Meddyproject/FilesystemUtils.h>
#include <MeddySDK/Meddydata/Utils.h>
#include <utility>
#include <optional>
#include <CommandParser.h>
#include <CommandParser/Utils.h>
#include <type_traits>
#include <CommandParser/FixedCapacityCstringConstant.h>
#include <algorithm>
#include <array>
#include <CppUtils/Core/Filesystem.h>
#include <MeddySDK/DAM/Utils.h>

// Command Synopsis Notes:
// Note: We use the command name "meta" to refer to "meddydata". This eliminates the repetitive appearance of the name "meddy" in "meddy meddydata" and
// is wayy easier to type. Comes at the cost of being slightly ambiguous about what type of metadata we are talking about (i.e, meddydata?, filesystem metadata?, embedded metadata?), but I
// think this is reasonably obvious. If we ever create new features built around other types of metadata, we can explictly specify for those. So the philosophy is: by default, we
// are refering to MEDDYdata whenever the term metadata is mentioned in the context of meddy.

// Each command node is defined by macro arguments in the format: (IndexName, ParentIndexName, LeafNameStringLiteral).
#define FOREACH_COMMAND(op) \
    op(project, NONE, "project") \
    op(project_init, project, "init") \
    op(project_current, project, "current") \
    op(meta, NONE, "meta") \
    op(meta_init, meta, "init")

#define COMMAND_INDEX_NAME_WITH_COMMA(IndexName, ParentIndexName, LeafNameStringLiteral) IndexName,
#define COMMAND_PARENT_INDEX_VALUE_WITH_COMMA(IndexName, ParentIndexName, LeafNameStringLiteral) MeddyCommandIndex::ParentIndexName,
#define COMMAND_STRING_LITERAL_WITH_COMMA(IndexName, ParentIndexName, LeafNameStringLiteral) LeafNameStringLiteral,

namespace MeddyCommandIndex
{
    constexpr auto NONE = CommandParser::InvalidCommandNodeIndex;

    enum : CommandParser::CommandNodeIndex
    {
        FOREACH_COMMAND(COMMAND_INDEX_NAME_WITH_COMMA)
        COUNT
    };
};

constexpr CommandParser::CommandNodeIndex MyCommandNodeParentArray[]
{
    FOREACH_COMMAND(COMMAND_PARENT_INDEX_VALUE_WITH_COMMA)
};

constexpr std::size_t MyCommandNodeNameStructSize{32};
constexpr alignas(std::min(MyCommandNodeNameStructSize, std::size_t{64u})) CommandParser::FixedCapacityCstringConstant<MyCommandNodeNameStructSize> MyCommandNodeNameArray[]
{
    FOREACH_COMMAND(COMMAND_STRING_LITERAL_WITH_COMMA)
};

static_assert(std::size(MyCommandNodeParentArray) == MeddyCommandIndex::COUNT, "Mismatch detected in generated command node arrays!");
static_assert(std::size(MyCommandNodeNameArray) == MeddyCommandIndex::COUNT, "Mismatch detected in generated command node arrays!");

// Undef these macros as they are no longer needed.
#undef FOREACH_COMMAND
#undef COMMAND_INDEX_NAME_WITH_COMMA
#undef COMMAND_PARENT_INDEX_VALUE_WITH_COMMA
#undef COMMAND_STRING_LITERAL_WITH_COMMA

// Note: As a CLI application, we ignore the first arg, as that is conventionally the program name, but it's not guarenteed to
// be anyway, and we don't need it anyway.
// @Christian: TODO: [todo][techdebt] Eliminate the error-handling code that emits user-facing messages into a
// separate library that handles SDK interaction and would potentially support localization as well.
int main(int argc, char** argv)
{
    std::cout << '\n';

    const std::filesystem::path cwdPathAbsolute = std::filesystem::current_path();
    assert(cwdPathAbsolute.is_absolute()); // The `current_path` function returns the path as an absolute.

    const CppUtils::CharBufferString<char, 2048> cwdPathAbsoluteCharBuffer =
        MeddySDK::ConstructGenericPathCharacterBuffer<2048, char>(
            std::filesystem::weakly_canonical(cwdPathAbsolute)
        );

    const std::string_view cwdPathAbsoluteString = cwdPathAbsoluteCharBuffer.ToStringView();

    CppUtils::ExpectedResult cwdMeddyproject = MeddySDK::GetOuterMeddyproject(std::filesystem::path{cwdPathAbsolute});

    assert(argc >= 0); // Must not be negative, because we are about to convert it to an unsigned size_t.

    CommandParser::ParsedCommand parsedCommand = CommandParser::ParseCommandIgnoringTheProgramNameTokenIndex(
        std::span{argv, static_cast<std::size_t>(argc)},
        std::span{MyCommandNodeNameArray, std::extent_v<decltype(MyCommandNodeNameArray)>},
        std::span{MyCommandNodeParentArray, std::extent_v<decltype(MyCommandNodeParentArray)>}
    );

    auto commandNodeFullName = CommandParser::GetFullNameOfCommandNode(parsedCommand.CommandNodeIndex,
        std::span{MyCommandNodeNameArray, std::extent_v<decltype(MyCommandNodeNameArray)>},
        std::span{MyCommandNodeParentArray, std::extent_v<decltype(MyCommandNodeParentArray)>}
    );

    // TODO: [todo] Implement validation that no additional, unrecognized arguments were given.
    switch (parsedCommand.CommandNodeIndex)
    {
    case MeddyCommandIndex::NONE:
        {
            if (parsedCommand.FlagArguments.size() >= 1)
            {
                if (parsedCommand.FlagArguments.contains("version"))
                {
                    std::cout << "TODO: Get ${PROJECT_VERSION} from CMake-generated header file." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                }
            }

            std::cout << "usage: meddy <command> [<args>]" << '\n';
            std::cout << '\n';
            std::cout << "Here are the basic meddy commands:" << '\n';
            std::cout << '\n';
            std::cout << "Create a project" << '\n';
            std::cout << "  meddy project init <project-root-dir>" << '\n';
            std::cout << '\n';
            std::cout << "Create meddydata" << '\n';
            std::cout << "  meddy meta init <source-pathname>" << '\n';

            std::cout << '\n';
            std::cout.flush();
            return 0;
        }
    case MeddyCommandIndex::project:
        {
            std::cout << "meddy: '" << commandNodeFullName << "' is not a fully specified command name." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy project init <project-root-dir>" << '\n';
            std::cout << "  meddy project current" << '\n';
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }
    case MeddyCommandIndex::project_init:
        {
            if (parsedCommand.PositionalArguments.size() < 1)
            {
                std::cout << "meddy: '" << commandNodeFullName << "' requires arguments." << '\n';
                std::cout << '\n';
                std::cout << "Accepted usage:" << '\n';
                std::cout << "  meddy project init <project-root-dir>" << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            const std::string_view projectRootPathArg = parsedCommand.PositionalArguments[0];

            std::filesystem::path projectRootPath = projectRootPathArg;

            MeddySDK::UncertainProjectCreationResult result =
                MeddySDK::TryCreateNewProject(std::move(projectRootPath));

            // TODO: [todo][techdebt] Support absolute paths as argument here as well.
            CppUtils::ExpectedResult projectRootPathAbsoluteResult = CppUtils::MakeAbsoluteUsingBase(projectRootPathArg, cwdPathAbsolute);
            if (projectRootPathAbsoluteResult.IsError())
            {
                // TODO: [todo][techdebt] Handle elegantly.
                assert(false);
                std::abort();
            }

            std::filesystem::path projectRootPathAbsolute = std::move(projectRootPathAbsoluteResult).GetValue();

            CppUtils::CharBufferString<char, 2048> projectRootPathAbsoluteCharBuffer =
                MeddySDK::ConstructGenericPathCharacterBuffer<2048, char>(
                    projectRootPathAbsolute
                );

            std::string_view projectRootPathAbsoluteString = projectRootPathAbsoluteCharBuffer.ToStringView();

            switch (result)
            {
            case MeddySDK::UncertainProjectCreationResult::Failed_ValidProjectAlreadyExists:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" is already a meddyproject." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_ProjectRootIsNonDirectory:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" is not a directory." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_ProjectRootDoesNotExist:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" is not an existing directory." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_MeddyprojectDirAlreadyExists:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" has an existing directory named \"" MEDDYSDK_MEDDYPROJECT_DIR_STRING_LITERAL "\" which is preventing this operation." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_MeddyprojectDirAlreadyExistsAndIsNonDirectory:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" has an existing file named \"" MEDDYSDK_MEDDYPROJECT_DIR_STRING_LITERAL "\" which is preventing this operation." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_FilesystemFailedToCreateMeddyprojectDir:
                std::cout << "error: Filesystem failed to create the \"" MEDDYSDK_MEDDYPROJECT_DIR_STRING_LITERAL "\"." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_FilesystemFailedToCreateManifestFile:
                std::cout << "error: Filesystem failed to create the \"" MEDDYSDK_MEDDYPROJECT_MANIFEST_FILENAME_STRING_LITERAL "\"." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            std::cout << "Successfully created project files. " << projectRootPathAbsolute.filename() << " is now a meddyproject." << '\n';
            std::cout << '\n';
            std::cout.flush();

            return 0;
        }
    case MeddyCommandIndex::project_current:
        {
            CppUtils::ExpectedResult result = MeddySDK::GetOuterMeddyproject(std::filesystem::path{cwdPathAbsolute});
            if (result.IsError())
            {
                switch (result.GetError())
                {
                case MeddySDK::Error_GetOuterMeddyprojectDirPath::PathDoesntExist:
                    std::cout << "error: \"" << cwdPathAbsoluteString << "\" does not exist." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                case MeddySDK::Error_GetOuterMeddyprojectDirPath::NoMeddyprojectDirFound:
                    std::cout << "No current meddyproject found." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                }

                std::cout << "error: Command failed for an unknown reason." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            CppUtils::CharBufferString<char, 2048> resultPathString =
                MeddySDK::ConstructGenericPathCharacterBuffer<2048, char>(
                    std::filesystem::weakly_canonical(std::move(result).GetValue().GetMeddyprojectDirPath())
                );

            std::cout << resultPathString.ToStringView() << '\n';
            std::cout << '\n';
            std::cout.flush();

            return 0;
        }
    case MeddyCommandIndex::meta:
        {
            std::cout << "meddy: '" << commandNodeFullName << "' is not a fully specified command name." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy meta init <source-pathname>" << '\n';
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }
    case MeddyCommandIndex::meta_init:
        {
            if (parsedCommand.PositionalArguments.size() < 1)
            {
                std::cout << "meddy: '" << commandNodeFullName << "' requires arguments." << '\n';
                std::cout << '\n';
                std::cout << "Accepted usage:" << '\n';
                std::cout << "  meddy meta init <source-pathname>" << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            const std::string_view sourcePathArg = parsedCommand.PositionalArguments[0];

            // TODO: [todo][techdebt] Support absolute paths as argument here as well.
            CppUtils::ExpectedResult sourcePathAbsoluteResult = CppUtils::MakeAbsoluteUsingBase(sourcePathArg, cwdPathAbsolute);
            if (sourcePathAbsoluteResult.IsError())
            {
                // TODO: [todo][techdebt] Handle elegantly.
                assert(false);
                std::abort();
            }

            std::filesystem::path sourcePathAbsolute = std::move(sourcePathAbsoluteResult).GetValue();

            // Use the cwd as the meddyproject to operate on. If the cwd is no meddyproject, then fall back on using the nearest (child-most) meddyproject of
            // the source file. This fallback behavior may change. TODO: [todo] Consider preventing this fallback behavior from happening or force the user
            // to be more explicit in order to avoid unexpected consequences in this occasion of abiguity.
            std::optional<MeddySDK::Meddyproject> meddyprojectToAddMeddydataTo;

            if (!cwdMeddyproject.IsError())
            {
                meddyprojectToAddMeddydataTo = cwdMeddyproject.GetValue();
            }
            else
            {
                CppUtils::ExpectedResult meddyprojectResult = MeddySDK::GetOuterMeddyproject(std::filesystem::path{sourcePathAbsolute});
                if (!meddyprojectResult.IsError())
                {
                    meddyprojectToAddMeddydataTo = std::move(meddyprojectResult).GetValue();
                }
                else
                {
                    switch (meddyprojectResult.GetError())
                    {
                    case MeddySDK::Error_GetOuterMeddyprojectDirPath::PathDoesntExist:
                        std::cout << "error: \"" << sourcePathAbsolute << "\" does not exist." << '\n';
                        std::cout << '\n';
                        std::cout.flush();
                        return 0;
                    case MeddySDK::Error_GetOuterMeddyprojectDirPath::NoMeddyprojectDirFound:
                        std::cout << "error: No outer meddyproject found." << '\n';
                        std::cout << '\n';
                        std::cout.flush();
                        return 0;
                    }

                    std::cout << "error: Command failed for an unknown reason." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                }
            }

            assert(meddyprojectToAddMeddydataTo.has_value());

            std::filesystem::path sourcePathRelativeToMeddyproject = sourcePathAbsolute.lexically_relative(meddyprojectToAddMeddydataTo->GetRootPath());

            CppUtils::ExpectedResult result =
                MeddySDK::AddMeddydata(*std::move(meddyprojectToAddMeddydataTo), std::move(sourcePathRelativeToMeddyproject));

            if (result.IsError())
            {
                switch (result.GetError())
                {
                case MeddySDK::Error_AddMeddydata::SourceFilePathContainsReservedMeddydataDirName:
                    std::cout << "error: Source file path contains reserved name \"" << MEDDYSDK_MEDDYDATA_DIR_STRING_LITERAL "\"." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                case MeddySDK::Error_AddMeddydata::FilesystemFailedToCreateMeddydata:
                    std::cout << "error: Filesystem failed to create the corresponding meddydata directory." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                case MeddySDK::Error_AddMeddydata::FilesystemFailedToCreateManifestFile:
                    std::cout << "error: Filesystem failed to create the \"" MEDDYSDK_MEDDYDATA_MANIFEST_FILENAME_STRING_LITERAL "\"." << '\n';
                    std::cout << '\n';
                    std::cout.flush();
                    return 0;
                }

                // Hits if there is an error returned that we don't have a case for in here.
                std::cout << "error: Command failed for an unknown reason." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            // TODO: [todo][plugins] Avoid directly calling on the DAM library here. Instead, we could have a plugin system and the MeddySDK_DAM library could be
            // a module library which gets callbacks invoked by dynamic library symbol export name. A plugin system like this implies many decisions to make
            // and many problems to solve though. E.g., should "plugins" be distributable as binaries and it should just plug-and-play work like that? With C++ that
            // would be tricky because of the not-stable ABI across different compilers. So third-party "Meddy plugin" developers would be forced to use the same
            // build toolchain as the original Meddy distributables. Maybe we don't care so much about plugins being user friendly though. I mean, they are
            // just C++ code that you would have to trust in the first place. Even if plugins used a scripting language, we would have to sandbox things and really
            // make sure it's locked down, and I don't want to attempt to do that. So maybe a plugin system can be useful only for simple cases like officially
            // developed optional Meddy plugins. It could also be useful for developers who want to extend off of Meddy to use it for their own software. E.g., if
            // you're writing a game engine and need to make an asset system which stores metadata about each asset, you could build that off of Meddy and write
            // a Meddy plugin to help accomplish that. Also, the idea of a plugin system pretty much implies that you are going to write a package manager with versioning
            // and dependencies, and that just sounds like a lot. Idk if we want to sign up for all of that.
            const bool successfullyAppendedDcmi = MeddySDK::AppendDcmiMetadataJson(result.GetValue());
            if (!successfullyAppendedDcmi)
            {
                std::cout << "error: Failed to write DCMI data to \"" MEDDYSDK_MEDDYDATA_MANIFEST_FILENAME_STRING_LITERAL "\"." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            CppUtils::CharBufferString<char, 2048> resultMeddydataPathString =
                MeddySDK::ConstructGenericPathCharacterBuffer<2048, char>(
                    std::filesystem::weakly_canonical(result.GetValue().GetMeddydataPath())
                );

            std::cout << "Successfully created meddydata at: " << resultMeddydataPathString.ToStringView() << '\n';
            std::cout << '\n';
            std::cout.flush();

            return 0;
        }
    default:
        assert(false); // Unimplemented command index.
    }

    return 0;
}

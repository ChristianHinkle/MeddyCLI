// Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

#include <iostream>
#include <string_view>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <MeddySDK/Meddyproject/Utils.h>
#include <MeddySDK/Meddyproject/FilesystemUtils.h>
#include <MeddySDK/Meddydata/Utils.h>
#include <utility>
#include <optional>
#include <CommandParser/ParseFunctions.h>
#include <CommandParser/Utils.h>
#include <type_traits>

namespace
{
    template <bool shouldFlush = true>
    void DumpHelpStdOutput();
}

// Note: [cache] Keeping this number as a power of two is important to keep element access straightforward (via simple offsetting & bit shifting) for the CPU, and to prevent
// the cstrings from straddling multiple cache lines unnecessarily.
constexpr std::size_t MyMaxCommandNodeNameCharLength{32};

// Note: [cache] An array of string views would be nicer obviously, but what we have here is the best for our use case, for a few reasons:
// 1. We have all our cstrings stored side-by-side in the same area of static storage, since it is a 2D character array.
//     - Side note: It's also nice that we have the strings spaced by a consistent alignment of `MyMaxCommandNodeNameLength`, which helps the CPU by making
//       predictable for the prefetcher and possibly the branch predictor as well for certain logic.
// 2. Accessing these cstrings is a simple memory offset. If we instead used string views, then that would be an array of pointers (and lengths), which would
//    require the CPU to use those pointers to jump to a completely different area of static storage, where the string literal is located.
// 3. Another negligible benefit is the space we're occupying in memory. A 2D array of characters is storing only the characters themselves, with offsets known at compile
//    time, whereas an array of `std::string_view`s would be actually storing pointers (and lengths) to the cstring literals, which would be taking up additional space.
//
// TODO: [todo] Improvement: Embed a precalculated length of the string into each element. We could change this an array of structs which store the char array and
// a separate length data member which occupies the space of a char, which means our max string length would be one less character after this change, as that last
// spot would now be used to store the string length.
constexpr char MyCommandNodeNameArray[][MyMaxCommandNodeNameCharLength]
{
    "project",
    "new",
    "current",
    "meddydata",
    "create",
};

// Note: [cache] Yes this is returning a new pointer (in the form of a string view) to the cstrings rather than using the array directly, but that does not imply a
// cost of indirection in this case. Because, if we compare these two cases:
// a) Accessing an element via the array directly: The resulting machine code has the offset to the data baked into the code that's using it.
// b) Accessing an element via a pointer returned by a function: The pointer is an offset to the data, which is not hardcoded into the machine code, but it is
//    a pointer stored in a CPU register or even on the stack, which located in a place of hot access and code execution, so we know it's already loaded into
//    cache at the very least. Also, the compiler will likely inline the implementation of this function, which will result in the offset being baked into the
//    machine code just like the other case.
//
// For an example of where this would be costly: If the pointer to the data was stored somewhere else (not on the stack/registers), then the CPU would first have to load the
// pointer itself into cache before even being able to use it to access the data.
constexpr std::string_view GetCommandNodeName(CommandParser::CommandNodeIndex index)
{
    // TODO: [todo] If we precalculate the length and store it along with these strings, we could directly use that here to construct our string view to
    // avoid the strlen calculation (which admittedly is already very fast considering specialized CPU instructions and the fact that these are fairly small string lengths).
    return MyCommandNodeNameArray[index];
}

constexpr CommandParser::CommandNodeIndex MyCommandNodeParentArray[]
{
    CommandParser::InvalidCommandNodeIndex,
    0u,
    0u,
    CommandParser::InvalidCommandNodeIndex,
    3u,
};

// Note: As a CLI application, we ignore the first arg, as that is conventionally the program name, but it's not guarenteed to
// be anyway, and we don't need it anyway.
// @Christian: TODO: [todo][techdebt] Eliminate the error-handling code that emits user-facing messages into a
// separate library that handles SDK interaction and would potentially support localization as well.
int main(int argc, char** argv)
{
    std::cout << '\n';

    const boost::filesystem::path cwdPathAbsolute = boost::filesystem::current_path();
    assert(cwdPathAbsolute.is_absolute()); // The `current_path` function returns the path as an absolute.

    const CppUtils::CharBufferString<char, 2048> cwdPathAbsoluteCharBuffer =
        MeddySDK::ConstructPrettyPathCharacterBuffer<2048, char>(
            boost::filesystem::weakly_canonical(cwdPathAbsolute)
        );

    const std::string_view cwdPathAbsoluteString = cwdPathAbsoluteCharBuffer.ToStringView();

    CppUtils::ExpectedResult cwdMeddyproject = MeddySDK::GetOuterMeddyproject(boost::filesystem::path{cwdPathAbsolute});

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
    case CommandParser::InvalidCommandNodeIndex:
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

            DumpHelpStdOutput();
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }
    case 0:
        {
            std::cout << "meddy: '" << commandNodeFullName << "' is not a fully specified command name." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy project new <project-root-dir>" << '\n';
            std::cout << "  meddy project current" << '\n';
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }
    case 1:
        {
            if (parsedCommand.PositionalArguments.size() < 1)
            {
                std::cout << "meddy: '" << commandNodeFullName << "' requires arguments." << '\n';
                std::cout << '\n';
                std::cout << "Accepted usage:" << '\n';
                std::cout << "  meddy project new <project-root-dir>" << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            const std::string_view projectRootPathArg = parsedCommand.PositionalArguments[0];

            boost::filesystem::path projectRootPath = projectRootPathArg;

            MeddySDK::UncertainProjectCreationResult result =
                MeddySDK::TryCreateNewProject(std::move(projectRootPath));

            boost::filesystem::path projectRootPathAbsolute = boost::filesystem::absolute(boost::filesystem::path{projectRootPathArg}, cwdPathAbsolute);

            CppUtils::CharBufferString<char, 2048> projectRootPathAbsoluteCharBuffer =
                MeddySDK::ConstructPrettyPathCharacterBuffer<2048, char>(
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
    case 2:
        {
            CppUtils::ExpectedResult result = MeddySDK::GetOuterMeddyproject(boost::filesystem::path{cwdPathAbsolute});
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
                MeddySDK::ConstructPrettyPathCharacterBuffer<2048, char>(
                    boost::filesystem::weakly_canonical(std::move(result).GetValue().GetMeddyprojectDirPath())
                );

            std::cout << resultPathString.ToStringView() << '\n';
            std::cout << '\n';
            std::cout.flush();

            return 0;
        }
    case 3:
        {
            std::cout << "meddy: '" << commandNodeFullName << "' is not a fully specified command name." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy meddydata create <source-pathname>" << '\n';
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }
    case 4:
        {
            if (parsedCommand.PositionalArguments.size() < 1)
            {
                std::cout << "meddy: '" << commandNodeFullName << "' requires arguments." << '\n';
                std::cout << '\n';
                std::cout << "Accepted usage:" << '\n';
                std::cout << "  meddy meddydata create <source-pathname>" << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            const std::string_view sourcePathArg = parsedCommand.PositionalArguments[0];
            boost::filesystem::path sourcePathAbsolute = boost::filesystem::absolute(boost::filesystem::path{sourcePathArg}, cwdPathAbsolute);

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
                CppUtils::ExpectedResult meddyprojectResult = MeddySDK::GetOuterMeddyproject(boost::filesystem::path{sourcePathAbsolute});
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

            boost::filesystem::path sourcePathRelativeToMeddyproject = sourcePathAbsolute.lexically_relative(meddyprojectToAddMeddydataTo->GetRootPath());

            CppUtils::ExpectedResult result =
                MeddySDK::AddMeddydata(*std::move(meddyprojectToAddMeddydataTo), std::move(sourcePathRelativeToMeddyproject));

            if (result.IsError())
            {
                switch (result.GetError())
                {
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

            CppUtils::CharBufferString<char, 2048> resultMeddydataPathString =
                MeddySDK::ConstructPrettyPathCharacterBuffer<2048, char>(
                    boost::filesystem::weakly_canonical(result.GetValue().GetMeddydataPath())
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

namespace
{
    template <bool shouldFlush>
    void DumpHelpStdOutput()
    {
        std::cout << "usage: meddy <command> [<args>]" << '\n';
        std::cout << '\n';
        std::cout << "Here are the basic meddy commands:" << '\n';
        std::cout << '\n';
        std::cout << "Create a project" << '\n';
        std::cout << "  meddy project new <project-root-dir>" << '\n';
        std::cout << '\n';
        std::cout << "Create meddydata" << '\n';
        std::cout << "  meddy meddydata create <source-pathname>" << '\n';

        if constexpr (shouldFlush)
        {
            std::cout.flush();
        }
    }
}

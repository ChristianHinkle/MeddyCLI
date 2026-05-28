// Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

#include <iostream>
#include <string_view>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <MeddySDK/Meddyproject/Utils.h>
#include <MeddySDK/Meddyproject/FilesystemUtils.h>
#include <MeddySDK/Meddydata/Utils.h>
#include <utility>

namespace
{
    template <bool shouldFlush = true>
    void DumpHelpStdOutput();
}

// @Christian: TODO: Dispatch commands in some kind of pipeline system, rather than using
// many hard-coded if statements in this main function.
// @Christian: TODO: Support both case of the executable path being passed as `argv[0]` as well as it
// not being passed at all.
// @Christian: TODO: [todo][techdebt] Eliminate the error-handling code that emits user-facing messages into a
// separate library that handles SDK interaction and would potentially support localization as well.
int main(int argc, char** argv)
{
    std::cout << '\n';

    if (argc <= 1)
    {
        std::cout << "try: meddy --help" << '\n';
        std::cout << '\n';
        std::cout.flush();
        return 0;
    }

    const boost::filesystem::path cwdPathAbsolute = boost::filesystem::current_path();
    assert(cwdPathAbsolute.is_absolute()); // The `current_path` function returns the path as an absolute.

    const CppUtils::CharBufferString<char, 2048> cwdPathAbsoluteCharBuffer =
        MeddySDK::ConstructPrettyPathCharacterBuffer<2048, char>(
            boost::filesystem::weakly_canonical(cwdPathAbsolute)
        );

    const std::string_view cwdPathAbsoluteString = cwdPathAbsoluteCharBuffer.ToStringView();

    CppUtils::ExpectedResult cwdMeddyproject = MeddySDK::GetOuterMeddyproject(boost::filesystem::path{cwdPathAbsolute});

    const std::string_view arg1st = argv[1];

    if (arg1st == "project")
    {
        if (argc <= 2)
        {
            std::cout << "meddy: '" << arg1st << "' requires arguments." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy project new <project-root-dir>" << '\n';
            std::cout << "  meddy project current" << '\n';
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }

        const std::string_view arg2nd = argv[2];
        if (arg2nd == "new")
        {
            if (argc <= 3)
            {
                std::cout << "meddy: '" << arg1st << " " << arg2nd << "' requires arguments." << '\n';
                std::cout << '\n';
                std::cout << "Possible commands" << '\n';
                std::cout << "  meddy project new <project-root-dir>" << '\n';
                std::cout << "  meddy project current" << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            // TODO: Error when extra args are given.

            const std::string_view arg3rd = argv[3];
            boost::filesystem::path projectRootPath = arg3rd;

            MeddySDK::UncertainProjectCreationResult result =
                MeddySDK::TryCreateNewProject(std::move(projectRootPath));

            boost::filesystem::path projectRootPathAbsolute = boost::filesystem::absolute(boost::filesystem::path{arg3rd}, cwdPathAbsolute);

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

        if (arg2nd == "current")
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

        std::cout << "meddy: '" << arg2nd << "' is not a " << arg1st << " command." << '\n';
        std::cout << '\n';
        std::cout << "Possible commands" << '\n';
        std::cout << "  meddy project new <project-root-dir>" << '\n';
        std::cout << "  meddy project current" << '\n';
        std::cout << '\n';
        std::cout.flush();
        return 0;
    }

    if (arg1st == "meddydata")
    {
        if (argc <= 2)
        {
            std::cout << "meddy: '" << arg1st << "' requires arguments." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy meddydata create <source-pathname>" << '\n';
            std::cout << '\n';
            std::cout.flush();
            return 0;
        }

        const std::string_view arg2nd = argv[2];
        if (arg2nd == "create")
        {
            if (argc <= 3)
            {
                std::cout << "meddy: '" << arg1st << " " << arg2nd << "' requires arguments." << '\n';
                std::cout << '\n';
                std::cout << "Possible commands" << '\n';
                std::cout << "  meddy meddydata create <source-pathname>" << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            // TODO: Error when extra args are given.

            const std::string_view arg3rd = argv[3];
            boost::filesystem::path sourcePathAbsolute = boost::filesystem::absolute(boost::filesystem::path{arg3rd}, cwdPathAbsolute);

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

        std::cout << "meddy: '" << arg2nd << "' is not a " << arg1st << " command." << '\n';
        std::cout << '\n';
        std::cout << "Possible commands" << '\n';
        std::cout << "  meddy meddydata create <source-pathname>" << '\n';
        std::cout << '\n';
        std::cout.flush();
        return 0;
    }

    if (arg1st == "--help")
    {
        // TODO: Error when extra args are given.
        DumpHelpStdOutput();
        std::cout << '\n';
        std::cout.flush();
        return 0;
    }

    if (arg1st == "--version")
    {
        // TODO: Error when extra args are given.
        std::cout << "TODO: Get ${PROJECT_VERSION} from CMake-generated header file." << '\n';
        std::cout << '\n';
        std::cout.flush();
        return 0;
    }

    std::cout << "meddy: '" << arg1st << "' is not a meddy command. See 'meddy --help'." << '\n';
    std::cout << '\n';
    std::cout.flush();
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

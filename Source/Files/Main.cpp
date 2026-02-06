// Copyright (c) 2023-2025 Christian Hinkle, Brian Hinkle.

#include <iostream>
#include <string_view>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <MeddySDK/Meddyproject/Meddyproject.h>
#include <MeddySDK/Meddyproject/FilesystemUtils.h>
#include <utility>

namespace
{
    template <bool shouldFlush = true>
    void DumpHelpStdOutput();
}

// TODO @Christian: Dispatch commands in some kind of pipeline system, rather than using
// many hard-coded if statements in this main function.
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

    const std::string_view arg1st = argv[1];

    if (arg1st == "project")
    {
        if (argc <= 2)
        {
            std::cout << "meddy: '" << arg1st << "' requires arguments." << '\n';
            std::cout << '\n';
            std::cout << "Possible commands" << '\n';
            std::cout << "  meddy project new <project-root-dir>" << '\n';
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
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            // TODO: Error when extra args are given.

            const std::string_view arg3rd = argv[3];
            boost::filesystem::path projectRootPath = arg3rd;

            MeddySDK::UncertainProjectCreationResult result =
                MeddySDK::TryCreateNewProject(std::move(projectRootPath));

            boost::filesystem::path projectRootPathAbsolute = boost::filesystem::absolute(boost::filesystem::path(arg3rd));

            CppUtils::CharBufferString<char, 2048> projectRootPathAbsoluteCharBuffer =
                MeddySDK::ConstructPrettyPathCharacterBuffer<2048, char>(
                    projectRootPathAbsolute);

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
            case MeddySDK::UncertainProjectCreationResult::Failed_DotMeddyprojectAlreadyExists:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" has an existing directory named \"" MEDDYSDK_DOT_MEDDYPROJECT_STRING_LITERAL "\" which is preventing this operation." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_DotMeddyprojectAlreadyExistsAndIsNonDirectory:
                std::cout << "error: \"" << projectRootPathAbsoluteString << "\" has an existing file named \"" MEDDYSDK_DOT_MEDDYPROJECT_STRING_LITERAL "\" which is preventing this operation." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_FilesystemFailedToCreateDotMeddyproject:
                std::cout << "error: Filesystem failed to create the \"" MEDDYSDK_DOT_MEDDYPROJECT_STRING_LITERAL "\"." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            case MeddySDK::UncertainProjectCreationResult::Failed_FilesystemFailedToCreateManifestFile:
                std::cout << "error: Filesystem failed to create the \"" MEDDYSDK_MANIFEST_FILENAME_STRING_LITERAL "\"." << '\n';
                std::cout << '\n';
                std::cout.flush();
                return 0;
            }

            std::cout << "Successfully created project files. " << projectRootPathAbsolute.filename() << " is now a meddyproject." << '\n';
            std::cout << '\n';
            std::cout.flush();

            return 0;
        }

        std::cout << "meddy: '" << arg2nd << "' is not a " << arg1st << " command." << '\n';
        std::cout << '\n';
        std::cout << "Possible commands" << '\n';
        std::cout << "  meddy project new <project-root-dir>" << '\n';
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

        if constexpr (shouldFlush)
        {
            std::cout.flush();
        }
    }
}

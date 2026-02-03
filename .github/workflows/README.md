# GitHub Actions Workflows

## Build Workflow (`build.yml`)

This workflow automatically builds the AutoRGB application for multiple platforms.

### Triggers
- **Push** to `main` or `master` branch
- **Pull Request** to `main` or `master` branch  
- **Manual trigger** via GitHub Actions UI (workflow_dispatch)

### Build Matrix
The workflow builds for three platforms:
- **x64** - 64-bit Intel/AMD processors
- **x86** - 32-bit Intel/AMD processors (legacy)
- **arm64** - ARM 64-bit processors (e.g., ARM-based Windows devices)

### Configuration
- **Build Configuration**: Release (optimized)
- **Build Tool**: MSBuild with Visual Studio 2022
- **Package Manager**: NuGet

### Build Steps
1. **Checkout code** - Downloads the repository
2. **Setup MSBuild** - Adds MSBuild to the PATH
3. **Setup NuGet** - Prepares NuGet package manager
4. **Restore packages** - Downloads all required NuGet dependencies
5. **Build solution** - Compiles the application for the target platform
6. **List output** - Shows what files were created (for debugging)
7. **Upload artifacts** - Packages and uploads the built application

### Artifacts
Build artifacts are automatically uploaded and can be downloaded from the GitHub Actions run page. Each platform build is packaged separately:
- `AutoRGBPrototype-x64-Release`
- `AutoRGBPrototype-x86-Release`
- `AutoRGBPrototype-arm64-Release`

### Output Location
Built files are typically located in:
- `AutoRGBPrototype/AutoRGBPrototype (Package)/AppPackages/`
- `AutoRGBPrototype/AutoRGBPrototype (Package)/{platform}/Release/`

### Using the Workflow

#### Automatic Builds
The workflow runs automatically on every push or pull request to the main/master branch.

#### Manual Builds
1. Go to the "Actions" tab in your GitHub repository
2. Select "Build AutoRGB Application" workflow
3. Click "Run workflow"
4. Select the branch and click "Run workflow"

#### Downloading Built Artifacts
1. Go to the "Actions" tab
2. Click on the completed workflow run
3. Scroll down to "Artifacts" section
4. Download the platform-specific zip file

### Requirements
This workflow requires:
- Windows runner (uses `windows-latest`)
- Access to Microsoft build tools and SDKs (provided by GitHub-hosted runners)
- NuGet packages defined in `packages.config`

### Troubleshooting

**Build fails with missing packages:**
- Check that `packages.config` lists all required NuGet packages
- Verify package versions are available

**Build fails with MSBuild errors:**
- Review the build logs in the GitHub Actions run
- Ensure the solution file and project files are properly configured

**Artifacts not uploading:**
- Check the "List build output" step to see what files were created
- Verify the paths in the "Upload build artifacts" step match actual output locations

### Customization

To modify the workflow:
1. Edit `.github/workflows/build.yml`
2. Change the `BUILD_CONFIGURATION` to `Debug` for debug builds
3. Modify the `matrix.platform` array to build only specific platforms
4. Adjust artifact paths if build output location changes

Example - Build only x64:
```yaml
strategy:
  matrix:
    platform: [x64]
```

Example - Build both Debug and Release:
```yaml
strategy:
  matrix:
    platform: [x64, x86, arm64]
    configuration: [Debug, Release]
```

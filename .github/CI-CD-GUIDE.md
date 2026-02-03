# CI/CD Build Process

## Overview
This document explains the automated build process for the AutoRGB application using GitHub Actions.

## Build Workflow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    TRIGGER EVENT                             │
│  (Push to main/master, PR, or Manual workflow dispatch)     │
└────────────────────┬────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────┐
│              GitHub Actions Runner (Windows)                 │
│                                                               │
│  ┌─────────────────────────────────────────────────┐        │
│  │  1. Checkout Code                                │        │
│  │     - Clone repository                           │        │
│  │     - Fetch all files and history                │        │
│  └────────────────┬─────────────────────────────────┘        │
│                   │                                           │
│                   ▼                                           │
│  ┌─────────────────────────────────────────────────┐        │
│  │  2. Setup Build Environment                      │        │
│  │     - Add MSBuild to PATH                        │        │
│  │     - Setup NuGet package manager                │        │
│  └────────────────┬─────────────────────────────────┘        │
│                   │                                           │
│                   ▼                                           │
│  ┌─────────────────────────────────────────────────┐        │
│  │  3. Restore Dependencies                         │        │
│  │     - Download NuGet packages                    │        │
│  │     - CppWinRT, WindowsAppSDK, WIL, etc.        │        │
│  └────────────────┬─────────────────────────────────┘        │
│                   │                                           │
│                   ▼                                           │
│  ┌─────────────────────────────────────────────────┐        │
│  │  4. Build Solution (Matrix Strategy)            │        │
│  │                                                  │        │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐     │        │
│  │  │  x64     │  │  x86     │  │  arm64   │     │        │
│  │  │ Release  │  │ Release  │  │ Release  │     │        │
│  │  └────┬─────┘  └────┬─────┘  └────┬─────┘     │        │
│  │       │             │             │            │        │
│  │       └─────────────┴─────────────┘            │        │
│  │                     │                           │        │
│  └────────────────────┬─────────────────────────────┘        │
│                       │                                       │
│                       ▼                                       │
│  ┌─────────────────────────────────────────────────┐        │
│  │  5. Package & Upload Artifacts                  │        │
│  │     - AutoRGBPrototype-x64-Release.zip          │        │
│  │     - AutoRGBPrototype-x86-Release.zip          │        │
│  │     - AutoRGBPrototype-arm64-Release.zip        │        │
│  └─────────────────────────────────────────────────┘        │
└───────────────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              Artifacts Available for Download                 │
│     (Available on GitHub Actions workflow run page)          │
└─────────────────────────────────────────────────────────────┘
```

## Build Matrix

The workflow uses a matrix strategy to build for multiple platforms in parallel:

| Platform | Description | Target Devices |
|----------|-------------|----------------|
| **x64** | 64-bit Intel/AMD | Most modern PCs |
| **x86** | 32-bit Intel/AMD | Legacy systems, 32-bit Windows |
| **arm64** | ARM 64-bit | ARM-based Windows devices (Surface Pro X, etc.) |

## Build Process Details

### Step 1: Checkout Code
```yaml
uses: actions/checkout@v4
```
- Clones the repository to the runner
- Includes all source files, project files, and configuration

### Step 2: Setup Build Environment
```yaml
uses: microsoft/setup-msbuild@v1.3
uses: nuget/setup-nuget@v1
```
- Adds MSBuild (Visual Studio build tool) to PATH
- Configures NuGet package manager

### Step 3: Restore Dependencies
```bash
nuget restore AutoRGBPrototype.sln
```
Downloads and installs:
- Microsoft.Windows.CppWinRT (2.0.230524.3)
- Microsoft.Windows.ImplementationLibrary (1.0.230411.1)
- Microsoft.Windows.SDK.BuildTools (10.0.22621.756)
- Microsoft.WindowsAppSDK (1.2.230313.1)
- Additional WindowsAppSDK components

### Step 4: Build Solution
```bash
msbuild AutoRGBPrototype.sln /p:Configuration=Release /p:Platform={platform}
```
Build parameters:
- `/p:Configuration=Release` - Optimized, production-ready build
- `/p:Platform={platform}` - Target architecture (x64/x86/arm64)
- `/m` - Multi-processor build (faster)
- `/p:AppxBundle=Never` - Don't create app bundle
- `/p:UapAppxPackageBuildMode=SideloadOnly` - Build for sideloading

Compiles:
1. AutoRGBPrototype.vcxproj (main application)
2. AutoRGBPrototype (Package).wapproj (package project)

### Step 5: Upload Artifacts
Packages and uploads:
- Compiled executable (.exe)
- Dependencies (.dll files)
- Package files (.msix or AppX)
- Debug symbols (.pdb) if included

Output locations:
- `AutoRGBPrototype/AutoRGBPrototype (Package)/AppPackages/`
- `AutoRGBPrototype/AutoRGBPrototype (Package)/{platform}/Release/`

## Workflow Triggers

### Automatic Triggers
1. **Push to main/master**
   ```yaml
   on:
     push:
       branches: [ main, master ]
   ```
   - Runs on every commit to main/master branch
   - Ensures main branch always has working builds

2. **Pull Requests**
   ```yaml
   on:
     pull_request:
       branches: [ main, master ]
   ```
   - Validates PR changes compile successfully
   - Runs before merging to main/master

### Manual Trigger
```yaml
on:
  workflow_dispatch:
```
- Allows manual workflow execution
- Useful for creating builds on-demand
- Accessible from GitHub Actions UI

## Accessing Build Artifacts

### From GitHub Actions UI
1. Navigate to repository → **Actions** tab
2. Click on the workflow run (e.g., "Build AutoRGB Application #123")
3. Scroll to **Artifacts** section at bottom of page
4. Download desired platform package:
   - `AutoRGBPrototype-x64-Release`
   - `AutoRGBPrototype-x86-Release`
   - `AutoRGBPrototype-arm64-Release`

### From Pull Request
- Build status shown as check on PR
- Click "Details" → View workflow run → Download artifacts

### From Commit
- Green checkmark ✓ indicates successful build
- Red X ✗ indicates build failure
- Click checkmark → Details → Artifacts

## Build Status Badge

The README includes a build status badge:

```markdown
[![Build Status](https://github.com/7effrey89/Dynamic-Lighting-AutoRGB/actions/workflows/build.yml/badge.svg)](https://github.com/7effrey89/Dynamic-Lighting-AutoRGB/actions/workflows/build.yml)
```

Badge states:
- 🟢 **Passing** - All builds succeeded
- 🔴 **Failing** - One or more builds failed
- 🟡 **No status** - Workflow hasn't run yet

## Build Output Structure

After successful build, artifacts contain:

```
AutoRGBPrototype-{platform}-Release/
├── AppPackages/
│   └── AutoRGBPrototype_1.0.0.0_{platform}_Test/
│       ├── AutoRGBPrototype.exe              # Main executable
│       ├── AutoRGBPrototype.msix             # Package file
│       ├── Dependencies/                      # Required DLLs
│       │   ├── {platform}/
│       │   │   ├── Microsoft.UI.Xaml.dll
│       │   │   ├── vcruntime140.dll
│       │   │   └── ...
│       └── Install.ps1                        # Installation script
└── {platform}/
    └── Release/
        ├── AutoRGBPrototype/                  # Build intermediate files
        └── AutoRGBPrototype (Package)/        # Package build output
```

## Troubleshooting

### Build Fails - Missing Packages
**Symptom:** NuGet restore fails or packages not found

**Solution:**
- Verify `packages.config` is correct
- Check package versions are available
- Review NuGet restore logs

### Build Fails - Compilation Errors
**Symptom:** MSBuild errors during compilation

**Solution:**
- Review error messages in workflow log
- Check if local build succeeds
- Verify all code changes committed
- Check for platform-specific issues

### Artifacts Not Found
**Symptom:** Upload artifacts step finds no files

**Solution:**
- Check "List build output" step to see what was created
- Verify build succeeded without errors
- Check artifact path patterns in workflow

### Workflow Doesn't Trigger
**Symptom:** Push/PR doesn't start workflow

**Solution:**
- Verify workflow file is in `.github/workflows/` directory
- Check YAML syntax is valid
- Ensure trigger branches match (main vs master)
- Check repository Actions are enabled

## Customization Examples

### Build Only x64
```yaml
strategy:
  matrix:
    platform: [x64]
```

### Add Debug Configuration
```yaml
strategy:
  matrix:
    platform: [x64, x86, arm64]
    configuration: [Debug, Release]
# Update build step to use ${{ matrix.configuration }}
```

### Change Trigger Branches
```yaml
on:
  push:
    branches: [ main, develop, release/* ]
```

### Add Build on Tag
```yaml
on:
  push:
    tags:
      - 'v*.*.*'
```

## Performance

Typical build times (parallel execution):
- **x64**: ~5-8 minutes
- **x86**: ~5-8 minutes  
- **arm64**: ~5-8 minutes

Total workflow time: ~8-10 minutes (due to parallel matrix)

## Cost

GitHub Actions provides free build minutes for public repositories:
- **Free tier**: Unlimited minutes for public repos
- **Private repos**: 2,000 minutes/month free, then paid

This workflow uses approximately 8-10 minutes per run.

## Next Steps

Potential enhancements:
- [ ] Add code signing for executables
- [ ] Create GitHub Releases automatically
- [ ] Add automated testing
- [ ] Generate changelogs
- [ ] Deploy to Microsoft Store
- [ ] Run security scans
- [ ] Generate documentation

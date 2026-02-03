# GitHub Actions Quick Reference

## 🚀 Quick Start

### View Build Status
**Main Page:** Check the badge on README.md  
**Actions Tab:** https://github.com/7effrey89/Dynamic-Lighting-AutoRGB/actions

### Download Latest Build
1. Go to [Actions](https://github.com/7effrey89/Dynamic-Lighting-AutoRGB/actions)
2. Click latest "Build AutoRGB Application" run
3. Scroll down to **Artifacts**
4. Download your platform:
   - `AutoRGBPrototype-x64-Release` (most common)
   - `AutoRGBPrototype-x86-Release` (32-bit)
   - `AutoRGBPrototype-arm64-Release` (ARM devices)

### Trigger Manual Build
1. Go to [Actions](https://github.com/7effrey89/Dynamic-Lighting-AutoRGB/actions)
2. Click "Build AutoRGB Application"
3. Click "Run workflow" button
4. Select branch
5. Click "Run workflow"

## 📋 Workflow Summary

| Aspect | Details |
|--------|---------|
| **Workflow Name** | Build AutoRGB Application |
| **File** | `.github/workflows/build.yml` |
| **Triggers** | Push to main/master, PRs, Manual |
| **Platforms** | x64, x86, arm64 |
| **Configuration** | Release (optimized) |
| **Build Time** | ~8-10 minutes |
| **Artifacts** | Downloadable ZIPs per platform |

## 🔧 Common Tasks

### Check Why Build Failed
```bash
# View in GitHub UI:
Actions → Click failed run → Click failed job → Expand failed step
```

### Re-run Failed Build
```bash
# In GitHub UI:
Actions → Click failed run → Click "Re-run all jobs"
```

### Test Changes Locally Before Push
```bash
# In Visual Studio:
Build → Build Solution (Ctrl+Shift+B)
# Or via command line:
msbuild AutoRGBPrototype.sln /p:Configuration=Release /p:Platform=x64
```

### View Build Logs
```bash
# In GitHub UI:
Actions → Click run → Click job → Expand steps
# Download logs: 
Actions → Click run → ⋮ menu → Download log archive
```

## 📁 Workflow Structure

```
.github/
├── workflows/
│   ├── build.yml          # Main workflow file
│   └── README.md          # Workflow documentation
└── CI-CD-GUIDE.md         # Comprehensive guide
```

## 🎯 Build Process Overview

```
Code Push → GitHub → Trigger Workflow → Windows Runner
  ↓
Setup Environment (MSBuild, NuGet)
  ↓
Restore NuGet Packages
  ↓
Build for x64, x86, arm64 (parallel)
  ↓
Upload Artifacts
  ↓
✓ Download from Actions page
```

## 📝 Modifying the Workflow

### Change Build Configuration
```yaml
env:
  BUILD_CONFIGURATION: Debug  # Change from Release
```

### Build Single Platform
```yaml
strategy:
  matrix:
    platform: [x64]  # Remove x86, arm64
```

### Add New Trigger
```yaml
on:
  push:
    branches: [ main, master, develop ]  # Add develop
```

## 🐛 Troubleshooting Quick Fixes

### Build Fails: Missing Packages
```bash
# Check packages.config is committed
# Verify package versions exist on NuGet
```

### Build Fails: Compilation Error
```bash
# Test locally first
# Check error in workflow logs
# Ensure all code changes are committed
```

### No Artifacts
```bash
# Check "List build output" step
# Verify build succeeded
# Check artifact paths in workflow
```

### Badge Not Updating
```bash
# Wait 5-10 minutes after workflow completes
# Clear browser cache
# Check workflow filename matches badge URL
```

## 🔗 Useful Links

- **Actions Tab**: https://github.com/7effrey89/Dynamic-Lighting-AutoRGB/actions
- **Workflow File**: `.github/workflows/build.yml`
- **Workflow Docs**: `.github/workflows/README.md`
- **CI/CD Guide**: `.github/CI-CD-GUIDE.md`
- **GitHub Actions Docs**: https://docs.github.com/en/actions

## 💡 Tips

- ✅ Always test builds locally before pushing
- ✅ Check build status badge before releasing
- ✅ Download artifacts to test before distribution
- ✅ Use "workflow_dispatch" for on-demand builds
- ✅ Review logs when builds fail
- ✅ Keep NuGet packages up to date

## 🎓 Learn More

- Workflow customization: `.github/workflows/README.md`
- Build process details: `.github/CI-CD-GUIDE.md`
- GitHub Actions: https://docs.github.com/actions
- MSBuild: https://docs.microsoft.com/visualstudio/msbuild/

---

**Need Help?** Check the full documentation:
- `.github/workflows/README.md` - Workflow usage
- `.github/CI-CD-GUIDE.md` - Detailed CI/CD guide

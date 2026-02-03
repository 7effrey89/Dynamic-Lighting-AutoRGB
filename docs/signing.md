# MSIX Package Signing Guide

This guide explains how to generate certificates, configure GitHub Actions secrets, and install signed MSIX packages.

## Overview

The GitHub Actions workflow automatically signs MSIX packages during the CI build process if certificate secrets are configured. Without signing, Windows will block installation of the MSIX packages.

**Important:** When signing is enabled, the workflow automatically exports the public certificate (`.cer` file) and includes it in the build artifacts alongside the signed MSIX packages. This makes it easy to install the certificate on target machines.

## Prerequisites

- Windows 10/11 with Developer Mode enabled (for development/testing)
- PowerShell (for certificate generation)
- Administrator access (for certificate installation)

## Generating a Self-Signed Certificate for Development

For development and testing purposes, you can create a self-signed certificate. **Note:** For production releases, you should use a certificate from a trusted Certificate Authority.

### Step 1: Create the Certificate

Open PowerShell as Administrator and run:

```powershell
# Create a self-signed certificate for code signing
$cert = New-SelfSignedCertificate `
    -Type Custom `
    -Subject "CN=YourName, O=YourOrganization, C=US" `
    -KeyUsage DigitalSignature `
    -FriendlyName "MSIX Signing Certificate" `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}")

# Display the certificate thumbprint
Write-Host "Certificate created with thumbprint: $($cert.Thumbprint)"
```

**Important:** Replace `YourName`, `YourOrganization`, and `C=US` with your actual details. 

**CRITICAL:** The `Subject` field **MUST exactly match** the `Publisher` field in your `AutoRGBPrototype/AutoRGBPrototype (Package)/Package.appxmanifest` file. 

Before creating the certificate, check the current Publisher in Package.appxmanifest:
```xml
<Identity
  Name="AutoRGB"
  Publisher="CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US"
  Version="1.0.0.0"/>
```

Either:
- **Option A**: Update the Publisher in Package.appxmanifest to match your certificate Subject (e.g., `CN=YourName, O=YourOrganization, C=US`)
- **Option B**: Create the certificate with a Subject that matches the existing Publisher in the manifest

The workflow will display both values during signing to help you verify they match.

### Step 2: Export the Certificate with Private Key (PFX)

Export the certificate as a PFX file with a password:

```powershell
# Set a strong password for the PFX file
$password = ConvertTo-SecureString -String "YourStrongPassword" -Force -AsPlainText

# Export the certificate with private key
$cert | Export-PfxCertificate `
    -FilePath "$env:USERPROFILE\Desktop\SigningCert.pfx" `
    -Password $password

Write-Host "Certificate exported to Desktop\SigningCert.pfx"
```

**Security Note:** Keep the PFX file and password secure. Never commit them to your repository.

### Step 3: Export the Public Certificate (CER)

Export the public certificate for installation on target machines:

```powershell
# Export public certificate (without private key)
$cert | Export-Certificate `
    -FilePath "$env:USERPROFILE\Desktop\SigningCert.cer" `
    -Type CERT

Write-Host "Public certificate exported to Desktop\SigningCert.cer"
```

## Configuring GitHub Actions Secrets

To enable automatic signing in the CI pipeline, you need to add the certificate as GitHub Actions secrets.

### Step 1: Convert PFX to Base64

Open PowerShell and run:

```powershell
# Read the PFX file and convert to Base64
$pfxPath = "$env:USERPROFILE\Desktop\SigningCert.pfx"
$pfxBytes = [System.IO.File]::ReadAllBytes($pfxPath)
$pfxBase64 = [System.Convert]::ToBase64String($pfxBytes)

# Copy to clipboard
$pfxBase64 | Set-Clipboard

Write-Host "Base64 string copied to clipboard. Length: $($pfxBase64.Length) characters"
```

Alternatively, save to a file:

```powershell
$pfxBase64 | Out-File "$env:USERPROFILE\Desktop\pfx_base64.txt" -NoNewline
Write-Host "Base64 string saved to Desktop\pfx_base64.txt"
```

### Step 2: Add Secrets to GitHub Repository

1. Navigate to your GitHub repository
2. Go to **Settings** → **Secrets and variables** → **Actions**
3. Click **New repository secret**
4. Add the following secrets:

   **Secret 1:**
   - Name: `WINDOWS_PFX_BASE64`
   - Value: Paste the Base64 string from Step 1

   **Secret 2:**
   - Name: `WINDOWS_PFX_PASSWORD`
   - Value: The password you used when exporting the PFX file (e.g., "YourStrongPassword")

5. Click **Add secret** for each

### Step 3: Verify Workflow Configuration

The workflow is automatically configured to:
- Build MSIX packages unsigned (using `/p:AppxPackageSigningEnabled=false`)
- Detect if secrets are present
- Sign all `.msix` files if secrets are available (post-build signing)
- Export the public certificate (`.cer`) and include it in the artifacts
- Skip signing with a clear message if secrets are not present
- Use SHA256 digest algorithm and DigiCert timestamp server

**Note:** Building packages unsigned and signing them post-build is the recommended approach for CI/CD pipelines, as it allows flexible certificate management without embedding certificates in the build process.

## Installing Signed MSIX Packages

To install a signed MSIX package on a target machine, you must first install the public certificate.

**Note:** The public certificate file (`AutoRGBPrototypeDev.cer`) is automatically included in the GitHub Actions artifacts when signing is enabled. Download the artifact and extract the certificate file.

### Option 1: Install via Double-Click (Easiest)

1. Download the artifact from GitHub Actions
2. Extract the `AutoRGBPrototypeDev.cer` file to the target machine
3. Double-click the `.cer` file
4. Click **Install Certificate**
5. Select **Local Machine** (requires Administrator) or **Current User**
6. Choose **Place all certificates in the following store**
7. Click **Browse** and select **Trusted People**
8. Click **OK** → **Next** → **Finish**

### Option 2: Install via PowerShell (Administrator)

```powershell
# Import certificate to Trusted People store (for sideloading)
$certPath = "C:\Path\To\AutoRGBPrototypeDev.cer"
Import-Certificate -FilePath $certPath -CertStoreLocation Cert:\LocalMachine\TrustedPeople

# Optionally, also add to Trusted Root (if needed)
Import-Certificate -FilePath $certPath -CertStoreLocation Cert:\LocalMachine\Root
```

### Option 3: Install via Command Prompt (Administrator)

```cmd
certutil -addstore TrustedPeople "C:\Path\To\AutoRGBPrototypeDev.cer"
```

### Installing the MSIX Package

After installing the certificate:

1. Locate the signed `.msix` file from GitHub Actions artifacts
2. Double-click the `.msix` file
3. Click **Install**
4. The app should install without certificate warnings

If you still see certificate errors, ensure:
- The certificate was installed in the correct store (**Trusted People** for sideloading)
- The certificate's Subject matches the Publisher in the app manifest
- The certificate has not expired

## Verifying Package Signature

To verify that an MSIX package is properly signed:

### Using PowerShell

```powershell
# Check signature status
$signature = Get-AuthenticodeSignature -FilePath "C:\Path\To\Package.msix"
Write-Host "Status: $($signature.Status)"
Write-Host "Signer: $($signature.SignerCertificate.Subject)"
Write-Host "Timestamp: $($signature.TimeStamperCertificate.Subject)"
```

Expected output for a signed package:
```
Status: Valid
Signer: CN=YourName, O=YourOrganization, C=US
Timestamp: CN=DigiCert Timestamp 2023, O=DigiCert, C=US
```

### Using Windows Explorer

1. Right-click the `.msix` file
2. Select **Properties**
3. Click the **Digital Signatures** tab
4. You should see your certificate listed

## Troubleshooting

### Issue: "This app package is not signed"

**Solution:** Ensure the GitHub Actions secrets are correctly configured and the workflow ran successfully.

### Issue: "The publisher could not be verified"

**Solution:** Install the public certificate (`.cer` file) in the **Trusted People** store on the target machine.

### Issue: "Certificate chain processed, but terminated in a root certificate which is not trusted"

**Solution:** For self-signed certificates, install the `.cer` file in both **Trusted People** AND **Trusted Root Certification Authorities** stores.

### Issue: Certificate mismatch error

**Solution:** Ensure the certificate's Subject field matches the Publisher field in `Package.appxmanifest`. You may need to update the manifest or regenerate the certificate.

### Issue: signtool.exe not found in workflow

**Solution:** The workflow now automatically searches for signtool.exe in multiple locations:
- SDK version directories (e.g., `10.0.22621.0/x64/signtool.exe`) for newer installations
- Direct architecture paths (e.g., `x64/signtool.exe`) for older installations
- Multiple architectures (x64, x86, arm64) as fallbacks

The Windows SDK should be pre-installed on `windows-latest` runners. If this error persists after the automatic search, verify the runner image has the Windows SDK installed.

### Issue: SignerSign() failed with error 0x8007000b (ERROR_BAD_FORMAT)

**Solution:** This error occurs when the certificate cannot sign the package. The workflow now displays diagnostic information to help identify the issue. Common causes:

1. **Certificate Subject Mismatch (Most Common)**: The certificate's Subject field must **exactly match** the Publisher field in `Package.appxmanifest`.
   - The workflow now displays both values during signing
   - Update either the manifest Publisher or regenerate the certificate to match
   - Example: If cert Subject is `CN=YourName, O=YourOrg, C=US`, the manifest Publisher must be exactly `CN=YourName, O=YourOrg, C=US`

2. **Incorrect Certificate Password**: Verify the `WINDOWS_PFX_PASSWORD` secret is correct
   - Test the password locally by opening the PFX file

3. **Invalid or Corrupted Certificate**: Re-export the PFX file and re-encode to Base64
   - Ensure the PFX file includes the private key

4. **Certificate Expired or Not Yet Valid**: Check the certificate validity dates
   - The workflow displays the certificate validity period

5. **Package Already Signed**: Ensure your workflow uses the `AppxPackageSigningEnabled=false` MSBuild parameter
   - Verify the package is not being signed during the build process

**To diagnose**: The workflow now automatically displays the certificate Subject and manifest Publisher during the signing step. Check the GitHub Actions logs for these details.

## Security Best Practices

1. **Never commit certificates:** Keep `.pfx` files and passwords out of source control
2. **Use strong passwords:** Protect PFX files with strong, unique passwords
3. **Limit secret access:** Only grant repository secrets access to trusted collaborators
4. **Rotate certificates:** Periodically regenerate certificates and update secrets
5. **Use trusted CAs for production:** Self-signed certificates are for development only
6. **Monitor secret usage:** Review GitHub Actions logs regularly (secrets are masked automatically)

## Additional Resources

- [Windows App Certification Kit](https://learn.microsoft.com/windows/uwp/debug-test-perf/windows-app-certification-kit)
- [Sign an app package using SignTool](https://learn.microsoft.com/windows/msix/package/sign-app-package-using-signtool)
- [Creating Certificates for Package Signing](https://learn.microsoft.com/windows/msix/package/create-certificate-package-signing)
- [GitHub Actions Encrypted Secrets](https://docs.github.com/actions/security-guides/encrypted-secrets)

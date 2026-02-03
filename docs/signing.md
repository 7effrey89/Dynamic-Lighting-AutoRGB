# MSIX Package Signing Guide

This guide explains how to generate certificates, configure GitHub Actions secrets, and install signed MSIX packages.

## Overview

The GitHub Actions workflow automatically signs MSIX packages during the CI build process if certificate secrets are configured. Without signing, Windows will block installation of the MSIX packages.

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

**Important:** Replace `YourName`, `YourOrganization`, and `C=US` with your actual details. The `Subject` field should match the `Publisher` field in your `Package.appxmanifest` file.

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
- Detect if secrets are present
- Sign all `.msix` files if secrets are available
- Skip signing with a clear message if secrets are not present
- Use SHA256 digest algorithm and DigiCert timestamp server

## Installing Signed MSIX Packages

To install a signed MSIX package on a target machine, you must first install the public certificate.

### Option 1: Install via Double-Click (Easiest)

1. Copy the `SigningCert.cer` file to the target machine
2. Double-click the `.cer` file
3. Click **Install Certificate**
4. Select **Local Machine** (requires Administrator) or **Current User**
5. Choose **Place all certificates in the following store**
6. Click **Browse** and select **Trusted People**
7. Click **OK** → **Next** → **Finish**

### Option 2: Install via PowerShell (Administrator)

```powershell
# Import certificate to Trusted People store (for sideloading)
$certPath = "C:\Path\To\SigningCert.cer"
Import-Certificate -FilePath $certPath -CertStoreLocation Cert:\LocalMachine\TrustedPeople

# Optionally, also add to Trusted Root (if needed)
Import-Certificate -FilePath $certPath -CertStoreLocation Cert:\LocalMachine\Root
```

### Option 3: Install via Command Prompt (Administrator)

```cmd
certutil -addstore TrustedPeople "C:\Path\To\SigningCert.cer"
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

**Solution:** The Windows SDK should be pre-installed on `windows-latest` runners. If this error occurs, contact GitHub support or verify the runner image version.

## Security Best Practices

1. **Never commit certificates:** Keep `.pfx` files and passwords out of source control
2. **Use strong passwords:** Protect PFX files with strong, unique passwords
3. **Limit secret access:** Only grant repository secrets access to trusted collaborators
4. **Rotate certificates:** Periodically regenerate certificates and update secrets
5. **Use trusted CAs for production:** Self-signed certificates are for development only
6. **Monitor secret usage:** Review GitHub Actions logs regularly (secrets are masked automatically)

## Additional Resources

- [Windows App Certification Kit](https://docs.microsoft.com/windows/uwp/debug-test-perf/windows-app-certification-kit)
- [Sign an app package using SignTool](https://docs.microsoft.com/windows/uwp/packaging/sign-app-package-using-signtool)
- [Creating Certificates for Package Signing](https://docs.microsoft.com/windows/msix/package/create-certificate-package-signing)
- [GitHub Actions Encrypted Secrets](https://docs.github.com/actions/security-guides/encrypted-secrets)

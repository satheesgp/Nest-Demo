# Nest SDK Demo — iOS

[![Platform](https://img.shields.io/badge/platform-iOS-lightgrey.svg)](#)
[![Swift](https://img.shields.io/badge/Swift-5.7%2B-orange.svg)](#)
[![iOS](https://img.shields.io/badge/iOS-15.0%2B-blue.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](#license)

A minimal iOS sample app showing how to authenticate, list devices, and toggle traits using a Nest-compatible API from an iPhone or iPad. It’s designed as a clean starting point for your own projects or for evaluating API access patterns (auth, fetching, command execution, and realtime updates).

> ℹ️ **About “Nest”**
> This demo is API-agnostic and can be wired to either:
>
> * **Google Nest Device Access (Smart Device Management / “SDM”)** via Google Cloud OAuth 2.0, or
> * A **custom “Nest SDK” wrapper** you maintain.
>   Update the `NestClient` implementation to match your SDK or REST endpoints.

---

## Table of Contents

* [Features](#features)
* [Screenshots](#screenshots)
* [Requirements](#requirements)
* [Project Structure](#project-structure)
* [Setup](#setup)

  * [1) Configure OAuth](#1-configure-oauth)
  * [2) Configure the App](#2-configure-the-app)
  * [3) Install Dependencies](#3-install-dependencies)
  * [4) Run](#4-run)
* [How It Works](#how-it-works)
* [Environment & Secrets](#environment--secrets)
* [Troubleshooting](#troubleshooting)
* [Extending the Demo](#extending-the-demo)
* [Contributing](#contributing)
* [License](#license)

---

## Features

* 👤 OAuth sign-in (PKCE) and secure token storage in Keychain
* 📋 Device list with basic traits (name, room, connectivity)
* 🔁 Pull-to-refresh and background refresh
* ⚙️ Device action examples (e.g., set mode/temperature where supported)
* 🧪 Unit tests for the API client layer
* 🧱 Clean architecture with protocol-driven SDK wrapper (`NestClient`)

---

## Screenshots

| Auth                          | Devices                       | Details                       |
| ----------------------------- | ----------------------------- | ----------------------------- |
| *(add your screenshots here)* | *(add your screenshots here)* | *(add your screenshots here)* |

---

## Requirements

* **Xcode** 14.3+
* **Swift** 5.7+
* **iOS** 15.0+ target (can raise/lower if you adjust concurrency/APIs)
* **CocoaPods** or **Swift Package Manager** (SPM)

If wiring to Google SDM:

* A **Google Cloud project** with **OAuth consent screen**, **iOS OAuth Client ID**, and **Device Access** set up.

---

## Project Structure

```
NestDemo/
├─ App/
│  ├─ NestDemoApp.swift          # App entry (SwiftUI)
│  ├─ SceneDelegate.swift        # (if you use UIKit lifecycle)
│  └─ Info.plist
├─ Features/
│  ├─ Auth/                      # OAuth flow & token storage
│  ├─ Devices/                   # List & details views
│  └─ Shared/
│     ├─ UI/                     # Reusable SwiftUI components
│     └─ Extensions/
├─ SDK/
│  ├─ NestClient.swift           # Protocol defining operations
│  ├─ NestClientLive.swift       # Concrete implementation
│  └─ Models.swift               # Device / Trait / Command models
├─ Tests/
│  ├─ NestClientTests.swift      # API client unit tests
│  └─ Mocks/
└─ Package.swift (if SPM) or Podfile (if CocoaPods)
```

---

## Setup

### 1) Configure OAuth

Create an OAuth client for iOS (using your provider—Google Cloud if targeting SDM):

* **Redirect URI (example):** `com.yourcompany.nestdemo:/oauth2redirect`

  * The scheme must match your bundle id or a custom scheme you control.
* **Scopes:** whatever your SDK requires (e.g., device read, control).
* **PKCE:** enabled.

Note your:

* **Client ID** (iOS)
* **Issuer/Discovery URL** (e.g., `https://accounts.google.com`)
* **Scopes**

### 2) Configure the App

Create a config file the app can read at build time:

**`Config.xcconfig`**

```xcconfig
NEST_CLIENT_ID = your-ios-oauth-client-id.apps.googleusercontent.com
NEST_REDIRECT_URI = com.yourcompany.nestdemo:/oauth2redirect
NEST_ISSUER = https://accounts.google.com
NEST_SCOPES = https://www.googleapis.com/auth/sdm.service
NEST_PROJECT_ID = your-gcp-project-id
NEST_AGENT_USER = your-agent-user-id
```

Then add the file to the “NestDemo” target and include it in **Build Settings → Swift Compiler – Custom Flags** as `Other Swift Flags` if you prefer, or read with a small helper from `Bundle.main.infoDictionary`. If you prefer environment files, create **`Secrets.plist`** and **do not commit it**.

Register the URL type in **Info.plist**:

```xml
<key>CFBundleURLTypes</key>
<array>
  <dict>
    <key>CFBundleURLSchemes</key>
    <array>
      <string>com.yourcompany.nestdemo</string>
    </array>
  </dict>
</array>
```

### 3) Install Dependencies

**Swift Package Manager (preferred)**
Open the project in Xcode and add packages:

* Your OAuth library (e.g., `AppAuth` via SPM mirror or a lightweight PKCE lib)
* Any HTTP client (URLSession is fine; no extra package required)

**CocoaPods (alternative)**

```bash
sudo gem install cocoapods
pod install
open NestDemo.xcworkspace
```

`Podfile` example:

```ruby
platform :ios, '15.0'
use_frameworks!
target 'NestDemo' do
  pod 'AppAuth', '~> 1.6'
end
```

### 4) Run

1. Open `NestDemo.xcodeproj` (or `.xcworkspace` if using Pods).
2. Select **NestDemo** scheme, choose a simulator or device.
3. **Cmd+R** to build & run.
4. Tap **Sign In**, complete OAuth, then view and control devices.

---

## How It Works

**Auth Flow**

```swift
let config = OAuthConfig(
  clientID: Env.clientID,
  issuer: URL(string: Env.issuer)!,
  redirectURI: URL(string: Env.redirectURI)!,
  scopes: Env.scopes
)

authController.startSignIn(from: presentingViewController) { result in
  switch result {
  case .success(let tokens):
    tokenStore.save(tokens) // stored in Keychain
  case .failure(let error):
    log.error("Auth failed: \(error)")
  }
}
```

**SDK Wrapper**

```swift
protocol NestClient {
  func listDevices() async throws -> [NestDevice]
  func getDevice(id: String) async throws -> NestDevice
  func sendCommand<DeviceCommand: Encodable>(
    deviceID: String,
    command: DeviceCommand
  ) async throws
}
```

**Fetching Devices**

```swift
let devices = try await nestClient.listDevices()
self.devices = devices.sorted(by: { $0.displayName < $1.displayName })
```

**Example Command**

```swift
try await nestClient.sendCommand(
  deviceID: device.id,
  command: ThermostatSetMode(mode: .heat)
)
```

Swap `NestClientLive` to your real implementation (Google SDM REST, WebSockets for updates, or your in-house SDK). The demo uses `URLSession` + async/await.

---

## Environment & Secrets

* Do **not** commit real credentials.
* Use `Config.xcconfig` or `Secrets.plist` and add to `.gitignore`:

```
# Secrets
Config.xcconfig
Secrets.plist
```

For CI, set `NEST_*` values as encrypted environment variables and inject them at build time.

---

## Troubleshooting

**Blank device list**

* Ensure the signed-in account has access and the proper scopes were granted.
* Verify `PROJECT_ID` / `AGENT_USER` (if applicable) and that your project is linked to the account.

**Auth loop or “No registered redirect URI”**

* The redirect URI in the Cloud console **must match** `NEST_REDIRECT_URI`.
* Check URL type registration in `Info.plist`.

**403 / Permission denied**

* Confirm Device Access permissions and that the OAuth consent screen is published (if required).
* Refresh tokens after changing scopes.

**Build fails on AppAuth**

* If mixing SPM and CocoaPods, prefer one. Clear derived data and re-resolve packages.

---

## Extending the Demo

* 🔔 **Realtime updates:** add gRPC/WebSocket listener if your SDK supports event streams.
* 🧩 **More traits:** support camera (live preview thumbnails), motion, occupancy.
* 🔒 **Biometric unlock:** protect device controls with Face ID / Touch ID.
* 🧪 **UI tests:** add snapshot tests for device states.
* 🌐 **Localization:** extract strings to `Localizable.strings`.

---

## Contributing

Pull requests welcome! Please:

1. Open an issue describing the change.
2. Keep PRs small and focused.
3. Add tests if you touch the SDK or parsing code.

---

## License

This project is licensed under the **MIT License**. See [LICENSE](./LICENSE) for details.

---

## Disclaimer

This repository is a **demo**. Use at your own risk and review your SDK’s terms, rate limits, and security requirements before shipping to production.

---

### Quick Start Checklist

* [ ] Create OAuth client (iOS)
* [ ] Fill `Config.xcconfig` / `Secrets.plist`
* [ ] Add URL scheme to `Info.plist`
* [ ] Choose dependency manager (SPM or CocoaPods)
* [ ] Implement `NestClientLive` for your SDK or API
* [ ] Run on device/simulator and sign in

---

If you want, paste your SDK/API specifics (endpoints, scopes, sample payloads) and I’ll tailor the `NestClientLive` stub and the README snippets exactly to your setup.

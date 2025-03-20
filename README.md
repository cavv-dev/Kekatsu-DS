# Kekatsu DS
Easy-to-use content downloader for Nintendo DS(i) consoles

![Screenshot 1](https://github.com/cavv-dev/Kekatsu-DS/raw/main/resources/screenshots/Kekatsu-DS_1.png) ![Screenshot 2](https://github.com/cavv-dev/Kekatsu-DS/raw/main/resources/screenshots/Kekatsu-DS_2.png) ![Screenshot 3](https://github.com/cavv-dev/Kekatsu-DS/raw/main/resources/screenshots/Kekatsu-DS_3.png)

The main scope of this project is to have a standalone and easy way to download apps and games on the fly, given a database by the user.

Concept inspired by [pkgi-psp](https://github.com/bucanero/pkgi-psp) and [Universal-Updater](https://github.com/Universal-Team/Universal-Updater).

## Features
- Multi-platform content download
- Database loading via URL or file
- Support for multiple databases
- ZIP file extraction
- Automatic updates check
- Customizable color scheme
- Localization support

## Quick setup instructions
1. Download `Kekatsu.nds` from the [latest release](https://github.com/cavv-dev/Kekatsu-DS/releases/latest).

2. Place `Kekatsu.nds` anywhere on your SD card.

3. Create a folder named `Kekatsu` in the root directory of your SD card. *(The "root" is the topmost directory of your SD card. See [this image](https://dsi.cfw.guide/assets/images/sdroot/en_US.png) for reference.)*

4. Create a text file named `databases.txt` in the `Kekatsu` folder.

5. **(Optional)** If you want to use the [UDB-Kekatsu-DS](https://github.com/cavv-dev/UDB-Kekatsu-DS) database, copy the following text into `databases.txt`:
    ```
    UDB-Kekatsu-DS	https://gist.githubusercontent.com/cavv-dev/3c0cbc1b63ac8ca0c1d9f549403afbf1/raw/
    ```
    Make sure that the "space" between the name and the URL is a **tab** character, not a space character.

6. Start *Kekatsu* on your system.

7. Navigate to the *Databases* menu by tapping its icon on the navbar. You should see the databases you added in the `databases.txt` file.

8. Tap on the database you want to use, and *Kekatsu* will load it.

## Databases setup
Kekatsu expects a `databases.txt` file to load as the list of available databases. This file has to be located in the `Kekatsu` directory on the root of the SD card.

Each line of the `databases.txt` file has to contain a name and a value separated by a tab character.

The name should be the display name of the database. The value must be either:
- an HTTP(S) URL which returns a database in text response
- a path to a database file on the SD card itself

### Example databases.txt
```
test-db https://example.com/database.txt
test-db2    /databases/db2.txt
```

## Community databases
Collection of useful known databases. Contact me to if you want to contribute to this list.

| Name | Description | URL |
|---|---|---|
| [UDB-Kekatsu-DS](https://github.com/cavv-dev/UDB-Kekatsu-DS) | Updated selection of DS and DSi apps from Universal-DB | `https://gist.githubusercontent.com/cavv-dev/3c0cbc1b63ac8ca0c1d9f549403afbf1/raw/` |

## Database creation instructions
A database file expected by Kekatsu is a text file that follows this precise structure:

- **Line 1**: Database version - The database version to be used by the parser. Follow the next instructions that match the chosen database version.

<details><summary>Version 1</summary>

- **Line 2**: Delimiter character - The character to be used to separate fields in the next lines
- **Line 3 and above**: Fields separated by the delimiter character. They must follow this order:
    - **Title** - Display title of the content
    - **Platform** - Target platform of the content. *Should* be in lowercase and in its abbreviated form as it will be used as the name of the platform directory. E.g. `nds` instead of `Nintendo DS`.
    - **Region** - Target region of the content. Could be `NTSC-U`, `PAL` and similar for contents which target a specific region or `ANY` for contents made for any region.
    - **Version** - Release version of the content
    - **Author** - Author or publisher of the content
    - **Download URL** - The HTTP(S) URL to download the content. Must be a direct link to the file of the content. This file can be an executable or an archive in ZIP format.
    - **File name** - The name under which the downloaded file will be saved
    - **Size** - The size in bytes of the downloaded file
    - **Box art URL** - The HTTP(S) URL of the displayed box art for the content. A box art is expected to be in PNG format.
    - **Extract items** - The items to be extracted from the downloaded archive in couples of fields separated by the delimiter character. Each couple is composed of:
        - **In-path** - The path of the file or directory in the archive to be extracted. Directories should have `/` as the last character.
        - **Out-path** - The destination path of the extracted file or directory
    
        If no extract items are specified, all the files and directories will be extracted following the structure in the archive.
        
        They are not going to be checked if the downloaded file is not an archive.

### Example database file
```
1
,
test-app,nds,ANY,1.0,Author1,https://example.com/test-app-v1.0.nds,test-app.nds,1048576,https://example.com/test-app-boxart.png
test-app2,gba,NTSC-U,1.1,Author2,https://example.com/test-app2.zip,test-app2.zip,2097152,https://example.com/test-app2-boxart.png,release/gba/test-app2-v1.1.gba,test-app2.gba
```

</details>

## Building
### Requirements
- devkitARM toolchain (`nds-dev` package group) by [devkitPro](https://github.com/devkitPro)
- [curl](https://github.com/ds-sloth/pacman-packages/tree/nds-curl-mbedtls/nds/curl) and [mbedtls](https://github.com/ds-sloth/pacman-packages/tree/nds-curl-mbedtls/nds/mbedtls) by [ds-sloth](https://github.com/ds-sloth)
- nds-zlib, nds-libpng from devkitPro pacman repository

```sh
git clone https://github.com/cavv-dev/Kekatsu-DS.git
cd Kekatsu-DS
make release
```

## Credits
- [Cavv](https://github.com/cavv-dev): Main developer
- [devkitPro](https://github.com/devkitPro): devkitARM and relative libraries
- [ds-sloth](https://github.com/ds-sloth): curl and mbedtls libraries
- [Flaticon](https://www.flaticon.com/): Icons

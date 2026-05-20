#include "RomLibrary.h"
#include "../../resources/NES_DB.h"

#include <openssl/evp.h>
#include <openssl/md5.h>

using namespace NS_Proteus;

void RomLibrary::Save() {
    // open or create our library file
    ofstream libraryFile(libraryPath);

    // create our json object
    json j;

    // for each pair in our `library` map...
    for (const pair<ConsoleID, vector<ROM_DATA>>& p : library) {
        // get a string version of our key
        string k = ConsoleNamesShort.at(p.first);

        // create an inner json object for our ROM_DATA vector
        json v;

        // for each ROM_DATA entry in our vector of ROM_DATA objects...
        for (const ROM_DATA& r : p.second) {
            // create an inner json object for our ROM_DATA entry
            json e;

            // populate our entry json object with the data from our ROM_ENTRY object
            e["gameName"] = r.gameName;
            e["path"] = r.path;
            e["hash"] = r.path;

            // add the entry json object to our vector json object
            v.push_back(e);
        }

        // now that all of our vectors have been json-ified,
        // add them to the main json object
        j[k] = v;
    }

    // now that the entire map has been json-ified,
    // serialize it into a proper json-file string
    string s = j.dump();
    // write the string to our file
    libraryFile.write(s.c_str(), s.length());
    // close the file
    libraryFile.close();
}

string RomLibrary::GetHash(const string& filepath) {
    // open file to compute hash from
    ifstream file(filepath, ios::binary);

    // This check should no longer be necessary;
    // `filepath` is obtained by locating the file,
    // and therefore should always exist by this point
    //
    // ensure file exists
    //if (!file.is_open()) {
    //    SDL_Log("Failed to open %s file for hashing!", filepath.c_str());
    //    return "";
    //}

    // init our hashing system
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);

    // create buffer to hold our file data
    char buffer[4096];
    // skip header bytes (I think?)
    file.seekg(16, file.beg);
    // read file into buffer
    while (file.read(buffer, sizeof(buffer))) {
        // for each byte read in, add it to the hash computation
        EVP_DigestUpdate(ctx, buffer, (size_t)file.gcount());
    }
    // include last byte of file in the hash computation
    EVP_DigestUpdate(ctx, buffer, (size_t)file.gcount());

    // init our hash result buffer
    u32 len = 0;
    u8 result[MD5_DIGEST_LENGTH];
    // compute the final hash into an array of bytes and fill our buffer
    // also, determine how many bytes are in the hash result
    EVP_DigestFinal_ex(ctx, result, &len);

    // free memory used by our hashing system
    EVP_MD_CTX_free(ctx);

    // convert our array of bytes into a string of hex byte values
    stringstream ss;
    // TODO: make sure using `len` instead of `MD5_DIGEST_LENGTH` doesn't break anything
    for (u32 i = 0; i < len; i++)
        ss << hex(result[i]);

    // return the final computed hash
    return ss.str();
}

string RomLibrary::Lookup(ConsoleID console, const string& hash) {
    // TODO: implement some way of finding data via internet

    // fallback to binary search of database file
    int left = 0;
    // TODO: use `console` to determine which database file we use
    int right = std::size(NES_ROM_DB) - 1;

    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = strcmp(hash.c_str(), NES_ROM_DB[mid].md5);

        if (cmp == 0) return NES_ROM_DB[mid].name;

        if (cmp < 0) right = mid - 1;
        else left = mid + 1;
    }

    return "Unknown";
}

void RomLibrary::Create() {
    // base path for location of ALL rom files
    const path base = "C:/ROMS/";
    if (!exists(base)) {
        // if the base doesn't exist, create it and return
        create_directory(base);
        return;
    }

    for (const auto& [key, value] : ConsoleNamesShort) { // for each console...
        Refresh(key);
    }
}

void RomLibrary::Load() {
    // if we don't already have a saved reference, create a new library
    // this allows us to only do the search/hash/create loop on first run
    if (!exists(libraryPath))
        return Create();

    // open our file for input
    ifstream f(libraryPath);

    // use provided `libraryPath` value to open our json file
    // and parse it into something processable.
    // this `j` object will become our `library` map
    json j = json::parse(f);

    // for each ConsoleID/vector pair `data` within `j`...
    for (json::iterator it = j.begin(); it != j.end(); it++) {
        if (it.value().is_null()) continue;
        // id had to be converted to a string for json format;
        // convert it back for use within our map
        ConsoleID id = GetIDFromName(it.key());

        // create a vector to hold the parse data from the file
        vector<ROM_DATA> v = {};

        // `e` will be the json format of each ROM_DATA entry
        for (const json& e : it.value())
            v.push_back({ .gameName = e["gameName"], .path = e["path"], .hash = e["hash"] });
        
        // now that we've filled the vector, simply add it to the library map
        library[id] = v;
    }
}

void RomLibrary::Refresh(ConsoleID console) {
    // TODO: Figure out a way to shorten processing time if the game being looked for is already in our list and valid

    path cPath = baseRomPath;
    // determine path to roms for THAT console
    string cName = ConsoleNamesShort.at(console);
    cPath /= cName;
    if (!exists(cPath)) {
        // if path doesn't exist, create it and continue
        create_directory(cPath);
        return;
    }
    // init our game vector
    vector<ROM_DATA> games = {};
    for (const auto& entry : directory_iterator(cPath)) { // for each file within the directory...
        // get text data
        string filepath = entry.path().string();
        string filename = entry.path().filename().string();

        // confirm that `filepath` leads to a file and not a directory
        if (!is_regular_file(filepath)) continue;

        // get hash
        string hash = GetHash(filepath);

        // get game name
        string gameName = Lookup(console, hash);
        // if we couldn't find gamename, default to using file name
        if (gameName == "Unknown") gameName = filename.substr(0, filename.find_last_of('.'));
        // add to our game list
        games.push_back({ .gameName = gameName, .path = filepath, .hash = hash });
    }
    library[console] = games;
}
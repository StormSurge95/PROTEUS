#include "RomLibrary.h"
#include "../../../resources/NES_DB.h"

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
            e["hash"] = r.hash;
            e["fileSize"] = r.fileSize;
            e["lastWrite"] = r.lastWrite;

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
    if (!exists(baseRomPath)) {
        // if the base doesn't exist, create it and return
        create_directory(baseRomPath);
        return;
    }

    for (const auto& [key, value] : ConsoleNamesShort) { // for each console...
        Refresh(key);
    }
}

void RomLibrary::Load() {
    // if we don't already have a saved reference, create a new library
    // this allows us to only do the search/hash/create loop on first run
    if (!exists(libraryPath)) {
        printf("%s does not exist; creating library from scratch...\n", libraryPath);
        Create();
    } else {
        // open our file for input
        printf("Opening %s file...\n", libraryPath);
        ifstream f(libraryPath);

        // use provided `libraryPath` value to open our json file
        // and parse it into something processable.
        // this `j` object will become our `library` map
        printf("Parsing data from library file...\n");
        json j = json::parse(f);

        // for each ConsoleID/vector pair `data` within `j`...
        printf("Starting outer for loop...\n");
        for (json::iterator it = j.begin(); it != j.end(); it++) {
            if (it.value().is_null()) {
                printf("null iterator value...\n");
                continue;
            }
            // id had to be converted to a string for json format;
            // convert it back for use within our map
            ConsoleID id = GetIDFromName(it.key());
            printf("Acquired ConsoleID: %d\n", id);

            // create a vector to hold the parse data from the file
            vector<ROM_DATA> v = {};

            // `e` will be the json format of each ROM_DATA entry
            printf("Starting inner for loop...\n");
            for (const json& e : it.value()) {
                string gName = e["gameName"];
                string fPath = e["path"];
                string hash = e["hash"];
                size_t size = e["fileSize"];
                u64 time = e["lastWrite"];
                v.push_back({ gName, fPath, hash, size, time });
            }
            

            // now that we've filled the vector, simply add it to the library map
            library[id] = v;
            Refresh(id);
        }
    }
}

void RomLibrary::Refresh(ConsoleID console) {
    path cPath = baseRomPath;
    // determine path to roms for THIS console
    string cName = ConsoleNamesShort.at(console);
    cPath /= cName;
    if (!exists(cPath)) {
        // if path doesn't already exist, create it and return
        create_directory(cPath);
        return;
    }

    vector<ROM_DATA>& currentList = library[console];
    unordered_map<string, size_t> idx;

    for (size_t i = 0; i < currentList.size(); i++) {
        string key = NormPathToKey(currentList[i].path);
        idx[key] = i;
    }

    vector<ROM_DATA> newList;
    set<path> seenKeys;

    for (const directory_entry& entry : directory_iterator(cPath)) {
        // make sure this entry is an actual file and not something else
        if (!is_regular_file(entry)) continue;

        string key = NormPathToKey(entry);
        seenKeys.insert(key);

        if (idx.contains(key)) {
            ROM_DATA old = currentList[idx[key]];
            if (Unchanged(old, entry))
                newList.push_back(old);
            else
                newList.push_back(NewData(console, entry));
        } else
            newList.push_back(NewData(console, entry));
    }

    library[console] = newList;
}

ROM_DATA RomLibrary::NewData(const ConsoleID id, const directory_entry& entry) {
    // get string data
    string fPath = entry.path().string();
    string fName = entry.path().filename().string();

    // get hash
    string hash = GetHash(fPath);

    // get game name
    string gName = Lookup(id, hash);

    // if game name couldn't be found, default to using file name
    if (gName == "Unknown") gName = fName.substr(0, fName.find_last_of('.'));

    size_t size = entry.file_size();
    u64 last = entry.last_write_time().time_since_epoch().count();

    return { gName, fPath, hash, size, last };
}

string RomLibrary::NormPathToKey(path raw) {
    path p = path(raw);
    string s = absolute(p).lexically_normal().generic_string();
    transform(s.begin(), s.end(), s.begin(), 
        [](u8 c) { return tolower(c); });
    return s;
}

bool RomLibrary::Unchanged(const ROM_DATA& oldData, const directory_entry& newData) {
    if (oldData.hash.empty())
        return false;

    if (NormPathToKey(oldData.path) != NormPathToKey(newData.path().string()))
        return false;

    if (oldData.fileSize != newData.file_size())
        return false;

    if (oldData.lastWrite != newData.last_write_time().time_since_epoch().count())
        return false;

    return true;
}
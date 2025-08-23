#include <bits/stdc++.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include <cstring>
#include <algorithm>
using namespace std;
/*Educational Image/file encryptor (xor stream with password-derived PRNG)
-Mode: encrypt |decrypt
-Header on encrypt files:
[8 bytes magic "ENCIMGO 1 "]
[8 bytes nonce(uint64_t little-endian)]*/
namespace util
{
    // splitMix64 - fast mixer (not cryotigraphic, just good diffusion)
    static inline uint64_t splitmix64(uint64_t x)
    {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    // combine password and nonce into a 64-bit seed (deterministic)
    uint64_t seed_from_password_nonce(const string &password, uint64_t nonce)
    {
        // FNV-1a 64-bit over password for stabiliyy across platforms
        const uint64_t FNV_OFFSET = 1469598103934665603ULL;
        const uint64_t FNV_PRIME = 1099511628211ULL;
        uint64_t h = FNV_OFFSET;
        for (unsigned char c : password)
        {
            h ^= c;
            h *= FNV_PRIME;
        }
        // Mix with nonce using splitmix64
        return splitmix64(h ^ splitmix64(nonce));
    }
    // Fill a buffer wih keystream bytes produced by mt19937_64
    void fill_keystream(mt19937_64 &prng, vector<unsigned char> &ks)
    {
        size_t i = 0, n = ks.size();
        while (i < n)
        {
            uint64_t r = prng(); // 8 bytes at a time
            size_t take = min<size_t>(8, n - i);
            memcpy(&ks[i], &r, take);
            i += take;
        }
    }
} // namespace util
static const char MAGIC[8] = {'E', 'N', 'C', 'I', 'M', 'G', 'O', '1'};
struct Args
{
    string mode; // encrypt | decrypt
    string in, out, pass;
};
bool parse_args(int argc, char **argv, Args &a)
{
    if (argc < 2)
        return false;
    a.mode = argv[1];
    for (int i = 2; i < argc; ++i)
    {
        string s = argv[i];
        auto need = [&](const char *opt) -> bool
        {
            if (i + 1 >= argc)
            {
                cerr << "Missing value for" << opt << "\n";
                return false;
            }
            return true;
        };
        if (s == "-i" || s == "--input")
        {
            if (!need("-i"))
                return false;
            a.in = argv[++i];
        }
        else if (s == "-o" || s == "-output")
        {
            if (!need("-o"))
                return false;
            a.out = argv[++i];
        }
        else if (s == "-p" || s == "--password")
        {
            if (!need("-p"))
                return false;
            a.pass = argv[++i];
        }
        else if (s == "-h" || s == "--help")
        {
            return false;
        }
        else
        {
            cerr << "Unknown option:" << s << "\n";
            return false;
        }
    }
    if ((a.mode != "encrypt" && a.mode != "decrypt") || a.in.empty() || a.out.empty() || a.pass.empty())
    {
        return false;
    }
    return true;
}
void usage(const char *prog)
{
    cerr << "Usage:\n"
            "  "
         << prog << "encrypt -i <input> -o <output.enc> -p <password>\n"
                    "  "
         << prog << "decrypt -i <input.enc> -o <output> -p <password>\n";
}
int main(int argc, char **argv)
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Args args;
    if (!parse_args(argc, argv, args))
    {
        usage(argv[0]);
        return 1;
    }
    constexpr size_t CHUNK = 1 << 20; // 1MB
    vector<unsigned char> inbuf(CHUNK), ks(CHUNK);
    if (args.mode == "encrypt")
    {
        ifstream fin(args.in, ios::binary);
        if (!fin)
        {
            cerr << "Cannot open input:" << args.in << "\n";
            return 1;
        }
        fin.seekg(0, ios::end);
        if (fin.tellg() <= 0)
        {
            cerr << "Input file is empty or unreadable: " << args.in << "\n";
            return 1;
        }
        fin.seekg(0, ios::beg);
        ofstream fout(args.out, ios::binary);
        if (!fout)
        {
            cerr << "Cannot open output:" << args.out << "\n";
            return 1;
        }
        uint64_t nonce;
        {
            random_device rd;
            nonce = (static_cast<uint64_t>(rd()) << 32) ^ rd();
        }
        // Write header
        fout.write(MAGIC, 8);
        fout.write(reinterpret_cast<const char *>(&nonce), sizeof(nonce));
        // Init PRNG
        uint64_t seed = util::seed_from_password_nonce(args.pass, nonce);
        mt19937_64 prng(seed);
        // Stream encrypt
        while (true)
        {
            fin.read(reinterpret_cast<char *>(inbuf.data()), inbuf.size());
            streamsize got = fin.gcount();
            if (got <= 0)
                break;
            ks.resize(static_cast<size_t>(got));
            util::fill_keystream(prng, ks);
            for (size_t i = 0; i < static_cast<size_t>(got); ++i)
                inbuf[i] ^= ks[i];
            fout.write(reinterpret_cast<const char *>(inbuf.data()), got);
        }
        cout << "Encrypted '" << args.in << " ' -> " << args.out << "'\n";
    }
    else
    { // decrypt
        ifstream fin(args.in, ios::binary);
        if (!fin)
        {
            cerr << "Cannot open input:" << args.in << "\n";
            return 1;
        }
        fin.seekg(0, ios::end);
        if (fin.tellg() <= 0)
        {
            cerr << "Input file is empty or unreadable: " << args.in << "\n";
            return 1;
        }
        fin.seekg(0, ios::beg);

        ofstream fout(args.out, ios::binary);
        if (!fout)
        {
            cerr << "Cannot open output:" << args.out << "\n";
            return 1;
        }
        // Read &check header
        char magic[8];
        uint64_t nonce = 0;

        // Read magic first
        fin.read(magic, sizeof(magic));
        if (!fin || !equal(begin(MAGIC), end(MAGIC), magic))
        {
            cerr << "Input does not look like an encrypted file (bad header)\n";
            return 1;
        }

        // Then read nonce
        fin.read(reinterpret_cast<char *>(&nonce), sizeof(nonce));
        if (!fin)
        {
            cerr << "Failed to read nonce from encrypted file\n";
            return 1;
        }
        // Init PRNG
        uint64_t seed = util::seed_from_password_nonce(args.pass, nonce);
        mt19937_64 prng(seed);
        // Stream decrypt (same XOR)
        while (true)
        {
            fin.read(reinterpret_cast<char *>(inbuf.data()), inbuf.size());
            streamsize got = fin.gcount();
            if (got <= 0)
                break;
            ks.resize(static_cast<size_t>(got));
            util::fill_keystream(prng, ks);
            for (size_t i = 0; i < static_cast<size_t>(got); ++i)
                inbuf[i] ^= ks[i];
            fout.write(reinterpret_cast<const char *>(inbuf.data()), got);
        }
        cout << "Decrypted'" << args.in << "' ->" << args.out << "'\n";
    }
    return 0;
}

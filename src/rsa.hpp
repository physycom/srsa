/* Copyright 2015-2017 - Nico Curti, Alessandro Fabbri */

/***************************************************************************
This file is part of srsa.
srsa is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
srsa is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with srsa. If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <random>

// key pair parameter obfuscation
// binary stream of Uint
// * * * N * * * E * D * * 
enum{
	KEY_LEN = 5,
	OFFS_N = 1,
	OFFS_E = 4,
	OFFS_D = 3
};

enum{
	MODE_ENCRYPT,
	MODE_DECRYPT
};

template<class Uint>
class RSA
{
public:
	Uint p, q, n, phi, e, d;

	RSA() {};
	// key pair generator constructor
	RSA(Uint p_, Uint q_) 
	{
		p = p_;
		if( !is_prime(p) ) throw std::runtime_error("p = " + std::to_string(p) + " is not a prime number");
		q = q_;
		if( !is_prime(q) ) throw std::runtime_error("q = " + std::to_string(q) + " is not a prime number");
		n = p_ * q_;
		if( n<255 )	throw std::runtime_error("n = " + std::to_string(n) + " is too small to support full ASCII table");
		phi = (p_ - 1) * (q_ - 1);
		compute_public_key(phi);
		compute_private_key(phi, e);
	}
	~RSA() = default;

	// support
	static bool is_prime(const Uint n_)
	{
		Uint i, j = (Uint) std::sqrt(n_);
		for (i = 2; i <= j; ++i)
			if (n_ % i == 0)
				return false;
		return true;
	};

	// init
	void compute_public_key(const Uint &phi_)
	{
		Uint i = 2;
		while (true){
			if (phi_ % i == 0){
				++i;
				continue;
			}
			if (is_prime(i)){
				e = i;
				break;
			}
			++i;
		}
	};

	void compute_private_key(const Uint &phi_, const Uint &e_)
	{
		Uint k = 1;
		while (true){
			k = k + phi_;
			if (k % e_ == 0){
				d = k / e_;
				break;
			}
		}
	};

	// keys
	void dump_keypair(std::string basename)
	{
		Uint keypub[KEY_LEN], keypri[KEY_LEN];
		// obfuscate key structure
		std::random_device rd;
    	std::mt19937 gen(rd());
    	std::uniform_int_distribution<Uint> dis;
		for(int i=0; i<KEY_LEN; ++i) keypub[i] = keypri[i] = dis(gen);
		keypub[OFFS_N] = keypri[OFFS_N] = n;
		keypub[OFFS_E] = e;
		keypri[OFFS_D] = d;
		// write binary files
		std::ofstream out(basename + ".pub", std::ios::binary);
		out.write((char*) keypub, sizeof(keypub));
		out.close();
		out.open(basename + ".rsa", std::ios::binary);
		out.write((char*) keypri, sizeof(keypri));
		out.close();
	}

	// utils
    template<class Uint_> friend std::ostream& operator<<(std::ostream& os, const RSA<Uint_> &rsa);
};

template<class Uint>
std::ostream& operator<<(std::ostream& os, const RSA<Uint>& rsa)
{  
    os << 
    	"RSA params " << std::endl <<
    	"p   = " << rsa.p << std::endl <<
    	"q   = " << rsa.q << std::endl <<
    	"n   = " << rsa.n << std::endl <<
    	"phi = " << rsa.phi << std::endl <<
    	"e   = " << rsa.e << std::endl <<
    	"d   = " << rsa.d << std::endl;
    return os;  
}

// encoder (public)
template<class Uint>
class RSAenc
{
public:
	Uint n, e;
	RSAenc(const Uint &n_, const Uint &e_) : n(n_), e(e_) {};
	RSAenc(std::string keyfile) 
	{
		std::ifstream in(keyfile, std::ios::binary);
		if( !in ) throw std::runtime_error("Key file : " + keyfile + " not found");
		std::vector<unsigned char> rawkey( (std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		Uint* key = (Uint*) &rawkey[0];
		n = key[OFFS_N];
		e = key[OFFS_E];
	}

	std::vector<Uint> encrypt(const std::string &msg)
	{
		std::vector<Uint> enc;
		Uint k, i;
		for (const auto &l : msg)
		{
			k = 1;
			Uint p = (Uint) l;
			for (i = 0; i < e; ++i) k = (k * p) % n;  // k = ( msg[i]^e ) % n
			enc.push_back(k);
		}
		return enc;
	}
};

// decoder (private)
template<class Uint>
class RSAdec{
public:
	Uint n, d;

	RSAdec(const Uint &n_, const Uint &d_) : n(n_), d(d_) {};
	RSAdec(std::string keyfile)
	{
		std::ifstream in(keyfile, std::ios::binary);
		if( !in ) throw std::runtime_error("Key file : " + keyfile + " not found");
		std::vector<unsigned char> rawkey( (std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		Uint* key = (Uint*) &rawkey[0];
		n = key[OFFS_N];
		d = key[OFFS_D];
	}

	std::string decrypt(const std::vector<Uint> &enc)
	{
		Uint k, i, j;
		std::string msg;
		for (const auto &l : enc)
		{
			k = 1;
			Uint c = (Uint) l;
			for (j = 0; j < d; ++j) k = (k * c) % n;
			msg.push_back((char)k);
		}
		return msg;
	}
};

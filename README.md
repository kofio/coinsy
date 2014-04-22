coinsy
======

An anonymous decentralized censorship resistant crypto-currency exchange.

I have implemented a realtime decentralized peer to peer exchange mechanism that is secure, anonymous, robust and fast. Packets are obfuscated and sensitive material is encrypted using public key cryptography.

The network can be easily extended to create other realtime services including decentralized market places or telephone systems.

The code can be compiled on most operating systems using bjam:

1. Download boost to deps/boost and deps/platforms/osx/boost.
2. Run deps/platforms/osx/boost/bootstrap.sh
3. To compile, something similar to:

	`bjam toolset=clang cxxflags="-std=c++11 -stdlib=libc++" linkflags="-stdlib=libc++" debug`

I have provided a reference implementation with a fully functioning client for Mac OS X. You can download it [here](http://hidemyass.com/files/1kC6c/).

There are several to do's in no particular order.

1. Implement a tcp specific protocol header with an N-bit length field to split incoming packets.
2. Implement a "Web of Trust" modal rather than a controlled decentralized public key infrastructure.
3. Implement block chain functionality.
4. Come to public consensus on escrow. A decentralized algorithm proves to work effectively in simulations but users may want to use centralized escrow services.
5. Use non-random account balances (related to 4).
6. Fix "hung" offers (possibly related to 1.).
7. Cleanup database code and upload.
8. Start implementing iOS, Android, Linux, Windows, etc clients.
10. Implement market chart into reference client for OS X.
11. Release version 1.0.

If you feel this software is of use please consider donating to one of the following crypto-currency addresses:

	BTC: 1CgoUnczfSetSTZxtY7oruSVxHAy8uQrRM
	NMC: NH6fRud4fWiVohwpJ7Z8qyu9HW1oYQqy1k
	LTC: LRBAC657e6HxBR2qLzHPq492pEiqppCd2J
	DOGE: D5G6FkhoQyNkvBBGnHpZNHAGUKtZ5FCUyN
	MINT: Mr23GFpyZbTHQ2Yi1xkBQCKb3V69EKRM7j
	VERT: Vof3tnHSFQsja92QySEgDTBYFMGvRv2Efz
	AUR: AbCykvY1G9hiMaaY13KYWsg3c3Kkv57ZF6
	HVC: HBxnfW8TGnS8JPDJ5QmyyN1yq6qbT2CW5e
	CON: UPM58usAn9QbdBNWJ5tKGiUQsfjaXJcyiM
	
If the address is not listed please let me know.

Adudalesdi Ganiladisdi

adudalesdi@hmamail.com

	-----BEGIN PGP PUBLIC KEY BLOCK-----
	
	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
	=xxdb
	-----END PGP PUBLIC KEY BLOCK-----

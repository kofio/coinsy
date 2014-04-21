coinsy
======

An anonymous decentralized censorship resistant crypto-currency exchange.

I have implemented a realtime decentralized peer to peer exchange mechanism that is secure, anonymous, robust and fast. Packets are obfuscated and sensitive material is encrypted using public key cryptography.

The network can be easily extended to create other realtime services including market places for physical items.

The code can be compiled on most operating systems using bjam, something similar to:

	`bjam toolset=clang cxxflags="-std=c++11 -stdlib=libc++" linkflags="-stdlib=libc++" debug`

I have provided a reference implementation with a fully functioning client for Mac OS X. You can download it [here](http://hidemyass.com/files/1kC6c/).

There are several to do's in no particular order.

1. Implement a tcp specific protocol header with an N-bit length field to split incoming packets.
2. Implement a "Web of Trust" modal rather than a controlled decentralized public key infrastructure.
3. Implement block chain functionality.
4. Come to public consensus on escrow. A decentralized algorithm proves to work effectively in simulations but users may want to use centralized escrow services.
5. Use non-random account balances.
6. Fix "hung" offers (possibly related to 1.).
7. Cleanup database code and upload.

If you feel this software is of use please consider donating to one of the following crypto-currency addresses:

	BTC: 1CgoUnczfSetSTZxtY7oruSVxHAy8uQrRM
	NMC: NH6fRud4fWiVohwpJ7Z8qyu9HW1oYQqy1k
	LTC: LRBAC657e6HxBR2qLzHPq492pEiqppCd2J
	DOGE: D5G6FkhoQyNkvBBGnHpZNHAGUKtZ5FCUyN
	MINT: Mr23GFpyZbTHQ2Yi1xkBQCKb3V69EKRM7j
	VERT: Vof3tnHSFQsja92QySEgDTBYFMGvRv2Efz
	
If the address is not listed please let me know.

adudalesdi@hmamail.com
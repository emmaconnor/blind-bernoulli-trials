# Blind Bernoulli Trials

A C implementation of [Blind Bernoulli Trials](https://www.usenix.org/conference/usenixsecurity19/presentation/connor), a non-interactive protocol for hidden weight coin flips.

## Building

This project requires the [Stanford Pairing-Based Crypto library](https://crypto.stanford.edu/pbc/), version 0.5.14. After installing this library, edit the Makefile to point `PBC_PREFIX` to the installation prefix of PBC. Then, type:

```
make
```

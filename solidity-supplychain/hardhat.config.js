require("@nomicfoundation/hardhat-toolbox");
require("dotenv").config();

/** @type import('hardhat/config').HardhatUserConfig */
module.exports = {
  solidity: {
    version: "0.8.20",
    settings: {
      optimizer: { // <-- ADD THIS WHOLE BLOCK
        enabled: true,
        runs: 200,
      },
      viaIR: true, 
    },
  },
  networks: {
    besu_private_net: {
      url: "http://localhost:8545",
      accounts: [process.env.PRIVATE_KEY_OWNER],
      chainId: 1337,
    },
  },
};
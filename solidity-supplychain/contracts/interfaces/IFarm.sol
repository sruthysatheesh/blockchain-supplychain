// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

interface IFarm {
    function createProduct(string memory _name, uint256 _quantity, string memory _unit) external;
    function splitAndShip(uint256 _sourceProductId, uint256 _quantity, address _destination) external;
}
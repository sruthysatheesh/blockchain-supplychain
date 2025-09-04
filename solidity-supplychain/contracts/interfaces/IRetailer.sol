// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

interface IRetailer {
    function receiveProduct(uint256 _productId) external;
    // Add any specific retailer functions here if needed, e.g., sell()
}
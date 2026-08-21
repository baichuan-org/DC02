export interface PoolConfig {
    id: string; // Unique identifier
    label: string; // Display name
    logo: string; // Filename in /public/pools/ or public URL
    value: string; // Stratum URL with port
    type: 'BTC' | 'LTC'; // Pool type for filtering
}

export const POOL_MODELS: PoolConfig[] = [
    // BTC Pools
    {
        id: 'ckpool',
        label: 'Solo CKPool',
        logo: '/pools/ckpool.svg',
        value: 'solo.ckpool.org:3333',
        type: 'BTC'
    },
    {
        id: 'miningdutch_btc',
        label: 'Mining-Dutch',
        logo: '/pools/miningdutch.png',
        value: 'sha256.mining-dutch.nl:9996',
        type: 'BTC'
    },
    {
        id: 'viabtc_btc',
        label: 'ViaBTC (Set Solo in Dashboard)',
        logo: '/pools/viabtc.svg',
        value: 'btc.viabtc.top:3333',
        type: 'BTC'
    },

    // LTC Pools
    {
        id: 'litesolo',
        label: 'LiteSolo',
        logo: '/pools/litesolo.svg',
        value: 'us.litesolo.org:3333',
        type: 'LTC'
    },
    {
        id: 'miningdutch_ltc',
        label: 'Mining-Dutch',
        logo: '/pools/miningdutch.png',
        value: 'scrypt.mining-dutch.nl:4083',
        type: 'LTC'
    },
    {
        id: 'viabtc_ltc',
        label: 'ViaBTC (Set Solo in Dashboard)',
        logo: '/pools/viabtc.svg',
        value: 'ltc.viabtc.top:3333',
        type: 'LTC'
    },
];

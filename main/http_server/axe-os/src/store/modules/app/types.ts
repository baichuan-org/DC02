import { MinerStatusData, ChartData, DomainData } from "@/api/type.ts";

export interface AppState {
  isAPMode: boolean;
  staticMenuDesktopInactive: boolean;
  staticMenuMobileActive: boolean;
  isDebugMode: boolean;
  windowInnerWidth: number;
  currentTime: Date;
  localTime: Date;
  deviceModel: string;
  statusRaw: MinerStatusData;

  dataLabel: number[];
  hashrateData: number[];
  temperatureData: number[];
  temperatureData2: number[]; // [新增] 第二块算力板温度
  powerData: number[];
  chartData: ChartData[];
  chartDataVersion: number;
  domainsOrigin: DomainData;
  domainsDst: DomainData;
  chartResetting: boolean;

  // WebSocket Global State
  ws: WebSocket | null;
  wsConnecting: boolean;
  wsConnected: boolean;
  logContent: string;

  needsRestart: boolean;

  isPollingPaused: boolean;

  // Auth & General
  token: string;
  isDataLoaded: boolean;
}
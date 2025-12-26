# hakoniwa-pdu-bridge

hakoniwa-pdu-bridge は、PDUチャネル間の論理的な橋渡し（bridge）を担う転送層コンポーネントです。
本リポジトリでは、箱庭PDUチャネル間の **転送ポリシーのみ** を定義します。

これにより、通信方式（TCP / UDP / WebSocket / Zenoh / SHM など）に依存せず、
- いつ転送するか
- どのPDUをどこへ流すか
を **論理的に制御** します。

---

## 転送ポリシー

PDU転送は、

- **更新に追従するか**
- **時間に従うか**

という2軸で整理できます。

箱庭では、その基本形として  
`immediate / throttle / ticker` の3つの転送ポリシーを採用します。

---

### immediate（即時反応）

更新したら必ず転送する。

- **定義**
  - PDUチャネルが更新／受信された瞬間に、可能な限り即座に転送する。
- **トリガー**
  - PDUチャネルの更新／受信タイミング
- **転送タイミング**
  - 更新／受信されたそのデータを即座に転送

主に、制御入力・状態同期など「遅延を許容しないデータ」に使用します。


---

### throttle（更新追従を間引く）

更新は追従するが、送信頻度を制限する。

- **定義**
  - PDUチャネルが更新／受信されても、指定された最小間隔を満たした場合のみ転送する。
- **トリガー**
  - PDUチャネルの更新／受信タイミング
- **転送タイミング**
  - 前回転送から一定時間以上経過している場合に、
    最新のデータを転送

高頻度センサやログ用途で、「情報量は落とさず流量を抑えたい」場合に使用します。

---


### ticker（定期転送）

時間になったら、今の状態を送る。

- **定義**
  - 一定周期で、PDUチャネルの現在値を転送する。
- **トリガー**
  - シミュレーション時間（周期タイマ）
- **転送タイミング**
  - 周期に達した時点で、PDUチャネルに存在する最新データを転送
  - ※ PDU更新がなくても転送は行われる

状態配信や可視化用途など、「現在値を定期的に知りたい」場合に使用します。


---

### 最小構成（immediate）

ノード１：
```json
{
  "node": "node1",
  "endpoints": {
    "local": { "file": "local_endpoint1.json" },
    "wire_tx_to_node2": { "file": "node1_wire_tx_to_node2.json" }
  },
  "bridges": [
    {
      "name": "node1->node2",
      "source_endpoint": "local",
      "dest_endpoints": ["wire_tx_to_node2"],
      "default_policy": { "type": "immediate" },
      "transfer_pdus": [
        { "robot_name": "Drone", "pdu_name": "pos" },
        { "robot_name": "Drone", "pdu_name": "status" }
      ]
    }
  ]
}

```

ノード２：側：
```json
{
  "node": "node2",
  "endpoints": {
    "wire_rx_from_node1": { "file": "node2_wire_rx_from_node1.json" },
    "local": { "file": "local_endpoint2.json" }
  },
  "bridges": [
    {
      "name": "node1->node2",
      "source_endpoint": "wire_rx_from_node1",
      "dest_endpoints": ["local"],
      "default_policy": { "type": "immediate" },
      "transfer_pdus": [
        { "robot_name": "Drone", "pdu_name": "pos" },
        { "robot_name": "Drone", "pdu_name": "status" }
      ]
    }
  ]
}
```

- endpoint は hakoniwa-pdu-endpoint で定義された通信端点設定(JSON)を指します。
- destinations は 1つ以上指定可能で、同一PDUを複数の通信端点へ fan-out できます。

---

### throttle の例

```json
{
  "policy": {
    "type": "throttle",
    "interval_ms": 100
  }
}
```

意味：

* PDUは更新され続けても
* **100ms以上経過したときだけ**
* 最新の値を転送する

---

### ticker の例

```json
{
  "policy": {
    "type": "ticker",
    "interval_ms": 50
  }
}
```

意味：

* 50ms周期で
* 現在PDUチャネルに存在する最新データを転送
* 更新がなくても送る

---


## 注意点


- 転送ポリシーは **通信方式とは独立** して定義されます
- src / dst の切り替えは Runtime Delegation と整合するように行われます
- 本リポジトリは「いつ・どう流すか」を扱い、「どう送るか（TCP/UDP/Zenoh等）」は protocol 層の責務です
---


# Factory Scanner 0.2.0 통합 기록

## 범위와 보존

- 원본 STEP/SLDASM/SLDPRT를 수정하지 않았다. STEP 조립 배치로 게임용 변환본을 생성했다.
- 기존 아이템 검색, 인벤토리 중복 제거, 결과 캐시, 연속 상대 방위각, 100ms 추적 및 GC 크래시 수정은 재사용한다.
- 새 지시서의 10초 표기는 이전 확정 요구와 충돌한다. 별도 답변이 없는 동안 기존 무제한 추적을 유지했다.
- 지시서는 Save/Load 항목 중간에서 끝난다. 확인 가능한 설정과 mode/range/dial을 함께 저장한다.

## CAD 변환

OpenCascade STEPCAF/XCAF로 7개 조립 인스턴스를 읽고 면 단위로 테셀레이션했다. Blender 설치 없이 처리했다.

- 원본 전체 크기: 300 × 200mm, 본체 기본 두께 30mm.
- 좌측 화면: 100 × 180mm. 우측 화면: 68 × 60mm.
- 본체의 원래 두 평면만 별도 메시로 분리. 나머지 형상 및 손잡이를 보존.
- 총 9개 메시, 16,348개 삼각형. 면 경계는 분리하고 각 곡면 안에서 법선을 재계산.
- 좌표: cm, +Y 오른쪽, +Z 위, -X 전면. 회전 피벗은 CAD의 원통 중심.
- 카테고리 스위치: 약 36.4도 양끝 전환. Mode: 약 53.46도. 약 0.1초 보간.
- 로터리: 30도/노치, 360도 순환하며 최단각 보간. 모델에 반복되는 홈을 기준으로 잡은 단계각이다.
- 화면 표시 면은 원래 표면보다 0.2mm 앞. DrawSize 540×972 및 544×480.
- 화면에는 플러그인 전용 unlit `SlateUI` 텍스처 재질을 사용해 주변 조명과 무관하게 UI를 표시.
- 짙은 청회색 본체, 청록 스위치, 주황 모드 스위치, 금속 회색 다이얼. 모든 메시 충돌 꺼짐.

재생성: `Tools/convert_scanner.py` → `Tools/build_equipment_assets.py` (Unreal Python). 종속 패키지는 작업 폴더 `.tools/cad` 안에만 설치했다.

## 코드 구성

- ItemScannerCatalog: 실제 RecipeManager 목록을 한 번 읽고 build descriptor를 제외하며 문화권별 FText 비교로 정렬.
- ItemScannerEquipment: AFGEquipment, Enhanced Input, 상태 저장/복제, 카메라 부착, 스위치 보간, 화면 갱신.
- ItemScannerEquipmentScreen: UUserWidget 소유 트리. UI 안에 검색 로직 없음.
- ItemScannerContent: Descriptor / Recipe / Unlock / Schematic / native GameWorldModule.
- ItemScannerConnectionScan: 기존 ExecuteScan 공통 진입점에 ConnectionCheck 분기 추가.
- Debug HUD는 기본 비활성. 켜면 종전 F7 기능 사용 가능.

## 연결 후보 기준과 한계

명시적 SCAN 시 한 번만 건물과 연결 컴포넌트를 읽는다. 30cm 공간 해시로 근접 쌍을 검사하고 거리순으로 중복 포트 쌍을 제거한다.

- 같은 매체, 다른 건물, 서로 호환되는 두 미연결 포트.
- normal dot ≤ -0.95, 두 포트가 간극을 향해 마주 봄.
- 벨트/파이프 본체 끝단이 적어도 하나 포함되어야 함.
- snap-only, 하이퍼튜브, 독립된 빈 포트는 제외.
- 물리적으로 가까운 의도적 미연결 쌍도 후보가 될 수 있음. UI에 CANDIDATE라고 명시.
- 추적은 저장된 약한 참조와 위치만 읽음. 수리 후 연결 여부/수량 확인에는 재스캔 필요.

## 검증 기록

- FactoryEditor Development 컴파일 성공.
- 최초 6개 NullRHI 자동 검사 통과.
- D3D12를 사용하는 8개 검사: 기존 3개 회귀 검사 + CAD/제작 데이터 + 선택 순환/연결 기하 + 장비 화면 GC + 화면 PNG 렌더 + 설정 필드 저장/복원.
- SDK의 `FFGDynamicStruct::InitializeAsRaw` 등은 비어 있어 실제 인벤토리 수명 주기를 에디터에서 검사할 수 없다. 설정 구조체는 SaveGame archive로 직접 왕복하여 target/mode/categories/range/dial을 검증한다. 게임의 인벤토리 저장/복원 검사는 별도다.
- Steam/Epic Win64 Shipping 빌드 및 Windows 패키징 성공. 메시·재질·아이콘 17개 에셋과 기본 설정 파일이 패키지에 포함됨.
- 실제 렌더링에서 5번째 행과 footer 겹침을 발견하여 4개/페이지로 수정.
- 두 화면의 실제 3D 장비 표면 표시를 확인. 촬영 검사는 첫 scene submission의 지연 텍스처 초기화 이후 한 프레임을 더 그려 검증한다. 독립 UI PNG만으로 성공을 판단하지 않는다.
- 3D 촬영 결과의 양쪽 화면 영역에서 밝은 글자 픽셀을 검사하여 검은 화면 회귀를 검출한다.
- 최종 보고서 및 렌더 파일: `release/tests-0.2.0-render/`, `release/preview-0.2.0/`.
- PNG의 제품명/결과는 레이아웃 검사용 fixture이며 실제 게임 검색 결과가 아니다.

## 게임 내 확인이 필요한 항목

2026-09-28 14:35 KST: 지정된 `D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows`에 0.2.0을 설치하고 패키지와 설치 파일 해시 일치를 확인했다. 게임 프로세스가 없는 상태에서 교체했다. 기존 0.1.4 전체 파일은 `release/installed-backup-0.1.4-20260928-143506`에 해시 검증하여 보관했다. 삭제한 파일은 없다.

1. Equipment Workshop에 Factory Scanner가 표시되고 정상 제작되는지.
2. 손 슬롯 장착 후 장비 및 두 화면 표시, 숫자키/휠/LMB가 정상 동작하는지.
3. 해제 후 핫바/장비 전환 입력이 원래대로 복구되는지.
4. 걷기/달리기/점프/FOV별 화면 위치와 손잡이 그립. 기본 portable-miner 양손 포즈는 임시 기준이며 전용 애니메이션이 아니다.
5. 새로고침/저장 후 장비를 다시 장착했을 때 선택 상태 복원.
6. 실제 벨트/파이프 오연결 사례와 정상 사용하지 않는 포트의 구별.
7. 원격 클라이언트/서버의 장착·입력·검색 데이터 완전성. 에디터 SDK의 FactoryGame 더미 구현으로는 멀티플레이 종단 검증이 불가능하다.

실제 게임을 자동 조작하거나 사용자의 세이브를 열어 검증하지 않았다. 이 항목을 완료했다고 주장하지 않는다.

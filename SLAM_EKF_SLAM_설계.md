# 알고리즘

## 변수
wheel_radius: 바퀴 반지름
lm rm : 좌우 모터의 속력을 받아와서 저장할 변수
plm prm : 이전 속력 값 (현재 속력을 저장하여 다음 동작후에 dlm,drm값을 구하는데 사용)
dlm drm : (lm – plm) * wheel_radius으로 현재 위치를 추정

## 동작
1. GetMotorPosition()함수를 사용하여 lm과dm에 모터의 속력을 저장
2. 가져온 현재 속력에 이전 속력을 빼고 바퀴사이 거리를 곱함
   = (llm – plm) * wheel_radius
3. predict()함수에 dlm,drm을 매개변수로 전달
- 변수
    axle_length : 이동길이 ?
    n 			: (X의 행개수 –3 )/2
    ds 			: dlm과 drm을 더하고 2로 나눔 = (dlm+drm)/2
    dtheta 	: (현재 오른쪽 모터 속력 – 현재 왼쪽 모터 속력)/이동길이
            	=(dlm-drm)/_ axel_length
		dMatrix Fv: 3,3+2*n을 저장함  
		th 			: X행렬의 2,0에 dtheta의 절반을 더함
    da 			: ds를 이동길이의 두배로 나눈다
    cos_th 	: theta의 cos값
    sin_th 	: theta의 sin값

- 동작
	- n을 설정
	- 3x(3+2*n)크기 행렬 FV설정
	- dMatrix U(3,1)
	 	- 3x1 크기의 행렬 U를 만듬


## Mapping
bit map
grid map 

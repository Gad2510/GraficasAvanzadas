Shader "Custom/Holograma_Surface"
{
    Properties
    {
        _Color ("Color", Color) = (1,1,1,1)
        _LineasColor("Lineas", Color) = (1,1,1,1)
        _Frecuencia("Frecuencia",Range(0.001,0.1)) = 0.5
        _Ondas("Frecuencia",float) = 10
        _AnchoOnda("Ancho Onda",float) = 5
        _Ancho("Ancho",Range(0.001,0.1)) = 0.5
        _MainTex ("Albedo (RGB)", 2D) = "white" {}
    }
    SubShader
    {
        Tags {"Queue" = "Transparent" "RenderType" = "Transparent" }
        LOD 200

        CGPROGRAM
        // Physically based Standard lighting model, and enable shadows on all light types
        #pragma surface surf Standard fullforwardshadows alpha:fade

        // Use shader model 3.0 target, to get nicer looking lighting
        #pragma target 3.0

        sampler2D _MainTex;

        struct Input
        {
            float2 uv_MainTex;
            float3 worldPos;
        };
        float _Frecuencia;
        float _Ondas;
        float _AnchoOnda;
        float _Ancho;
        fixed4 _Color;
        fixed4 _LineasColor;

        // Add instancing support for this shader. You need to check 'Enable Instancing' on materials that use the shader.
        // See https://docs.unity3d.com/Manual/GPUInstancing.html for more information about instancing.
        // #pragma instancing_options assumeuniformscaling
        UNITY_INSTANCING_BUFFER_START(Props)
            // put more per-instance properties here
        UNITY_INSTANCING_BUFFER_END(Props)

        void surf (Input IN, inout SurfaceOutputStandard o)
        {
            float2 uv = IN.uv_MainTex;
            uv.x = uv.x + (_Time.x * 2) + (sin((uv.y + _Time.x) * _Ondas) / _AnchoOnda);

            fixed4 lineas = _LineasColor;
            fixed3 col = lerp(lineas.rgb,_Color.rgb,abs(sin((IN.worldPos.y+ _Time.x) * 200)));

            float2 uvMove = float2(IN.uv_MainTex.x+_Time.x, IN.uv_MainTex.y);
            fixed4 noise = tex2D(_MainTex, uvMove);
            float2 uvNoise = float2(IN.uv_MainTex.x+ noise.x, IN.uv_MainTex.y+ noise.x);
            fixed3 tex = tex2D(_MainTex, uvNoise).rgb * col;

            // Albedo comes from a texture tinted by color
            o.Albedo = col.rgb * tex.rgb;
            // Metallic and smoothness come from slider variables
            o.Alpha = clamp(tex.r,0.2,1);
            
        }
        ENDCG
    }
    FallBack "Diffuse"
}

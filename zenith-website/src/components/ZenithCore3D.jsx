import React, { useRef, useState } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { 
  Float, 
  MeshDistortMaterial, 
  MeshWobbleMaterial, 
  PerspectiveCamera, 
  PresentationControls,
  MeshTransmissionMaterial,
  Environment
} from '@react-three/drei';
import * as THREE from 'three';

const ZenithShape = () => {
  const mesh = useRef();
  const [hovered, setHovered] = useState(false);

  useFrame((state) => {
    const t = state.clock.getElapsedTime();
    mesh.current.rotation.x = THREE.MathUtils.lerp(mesh.current.rotation.x, Math.cos(t / 2) / 4 + 0.25, 0.1);
    mesh.current.rotation.y = THREE.MathUtils.lerp(mesh.current.rotation.y, Math.sin(t / 4) / 4, 0.1);
    mesh.current.rotation.z = THREE.MathUtils.lerp(mesh.current.rotation.z, Math.sin(t / 8) / 4, 0.1);
    mesh.current.position.y = THREE.MathUtils.lerp(mesh.current.position.y, (Math.sin(t / 2) / 4) + (hovered ? 0.2 : 0), 0.1);
  });

  return (
    <group>
      {/* Outer Shell - Glassy Dodecahedron */}
      <mesh 
        ref={mesh} 
        onPointerOver={() => setHovered(true)} 
        onPointerOut={() => setHovered(false)}
      >
        <dodecahedronGeometry args={[2.2, 0]} />
        <MeshTransmissionMaterial 
          backside 
          samples={16} 
          thickness={1} 
          chromaticAberration={0.02} 
          anisotropy={0.1} 
          distortion={0.1} 
          distortionScale={0.2} 
          temporalDistortion={0.1} 
          irisBlur={0.5}
          ior={1.2}
          color="#ffffff"
        />
      </mesh>

      {/* Inner Core - Glowing Sphere */}
      <Float speed={2} rotationIntensity={1} floatIntensity={2}>
        <mesh position={[0, 0, 0]}>
          <sphereGeometry args={[0.8, 32, 32]} />
          <meshStandardMaterial 
            color="#0071e3" 
            emissive="#0071e3" 
            emissiveIntensity={2} 
            toneMapped={false} 
          />
        </mesh>
      </Float>

      {/* Internal Geometry Details */}
      <mesh rotation={[Math.PI / 4, 0, 0]}>
        <torusGeometry args={[1.5, 0.02, 16, 100]} />
        <meshBasicMaterial color="#0071e3" transparent opacity={0.3} />
      </mesh>
    </group>
  );
};

const ZenithCore3D = () => {
  return (
    <div className="w-full h-full min-h-[400px]">
      <Canvas shadows dpr={[1, 2]} camera={{ position: [0, 0, 10], fov: 35 }}>
        <PerspectiveCamera makeDefault position={[0, 0, 8]} fov={40} />
        <Environment preset="city" />
        
        <ambientLight intensity={0.5} />
        <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} intensity={1} castShadow />
        <pointLight position={[-10, -10, -10]} intensity={0.5} color="#0071e3" />

        <PresentationControls
          global
          config={{ mass: 2, tension: 500 }}
          snap={{ mass: 4, tension: 1500 }}
          rotation={[0, 0.3, 0]}
          polar={[-Math.PI / 3, Math.PI / 3]}
          azimuth={[-Math.PI / 1.4, Math.PI / 1.4]}
        >
          <ZenithShape />
        </PresentationControls>
        
        <ContactShadows position={[0, -3.5, 0]} opacity={0.4} scale={10} blur={2} far={4} />
      </Canvas>
    </div>
  );
};

function ContactShadows(props) {
  return (
    <mesh rotation={[Math.PI / 2, 0, 0]} {...props}>
      <planeGeometry />
      <meshBasicMaterial color="#000000" transparent opacity={0.1} />
    </mesh>
  );
}

export default ZenithCore3D;

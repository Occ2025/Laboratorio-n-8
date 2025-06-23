from mido import MidiFile
import os

## ================= CONFIGURACIÓN DE RUTAS ================= ##
# Ruta base donde se guardará el archivo .ino (MODIFICABLE SI ES NECESARIO)
RUTA_BASE = "D:\Mis Documentos\Downloads\Lab8Circuit1"

# Nombre del archivo MIDI de entrada (cambiar si es diferente)
ARCHIVO_MIDI = "Little_Fugue_In_G_Minor.mid"

# Nombre del archivo Arduino de salida
ARCHIVO_ARDUINO = "melodia_arduino.ino"

## ================= CONSTRUCCIÓN DE RUTAS COMPLETAS ================= ##
ruta_midi = os.path.join(RUTA_BASE, ARCHIVO_MIDI)
ruta_salida = os.path.join(RUTA_BASE, ARCHIVO_ARDUINO)

## ================= VERIFICACIONES INICIALES ================= ##
print(f"\n=== Sistema de conversión MIDI a Arduino ===")
print(f"\nRuta base configurada: {RUTA_BASE}")

# Verificar si existe la carpeta base
if not os.path.exists(RUTA_BASE):
    print(f"\nERROR: La carpeta no existe: {RUTA_BASE}")
    print("\nPor favor crea la carpeta o corrige la ruta en el script")
    exit()

# Verificar archivo MIDI
if not os.path.exists(ruta_midi):
    print(f"\nERROR: Archivo MIDI no encontrado: {ruta_midi}")
    print("\nArchivos MIDI disponibles en la carpeta:")
    midis = [f for f in os.listdir(RUTA_BASE) if f.lower().endswith(('.mid', '.midi'))]
    
    if midis:
        for m in midis:
            print(f"- {m}")
    else:
        print("No se encontraron archivos .mid o .midi")
    
    print("\nSolución:")
    print(f"1. Coloca tu archivo MIDI en: {RUTA_BASE}")
    print(f"2. Verifica el nombre en ARCHIVO_MIDI (actual: '{ARCHIVO_MIDI}')")
    exit()

## ================= PROCESAMIENTO MIDI ================= ##
print(f"\nProcesando archivo MIDI: {ARCHIVO_MIDI}")

note_freqs = {
    60: 262, 61: 277, 62: 294, 63: 311, 64: 330, 65: 349,
    66: 370, 67: 392, 68: 415, 69: 440, 70: 466, 71: 494,
    72: 523, 73: 554, 74: 587, 75: 622, 76: 659, 77: 698,
    78: 740, 79: 784, 80: 831, 81: 880, 82: 932, 83: 988,
    84: 1047
}

try:
    midi = MidiFile(ruta_midi)
    tempo = 500000
    ticks_per_beat = midi.ticks_per_beat

    melody = []
    durations = []

    for track in midi.tracks:
        for msg in track:
            if msg.type == 'set_tempo':
                tempo = msg.tempo
            elif msg.type == 'note_on' and msg.velocity > 0:
                note = msg.note
                if note in note_freqs:
                    duration_ms = (msg.time * tempo) / (ticks_per_beat * 1000)
                    melody.append(note_freqs[note])
                    durations.append(int(duration_ms) if duration_ms > 0 else 100)

    # Limitar a 100 notas para prueba
    melody = melody[:100]
    durations = durations[:100]

    ## ================= GENERACIÓN DEL ARCHIVO ARDUINO ================= ##
    print(f"\nGenerando archivo Arduino en: {ruta_salida}")
    
    try:
        with open(ruta_salida, 'w') as f:
            f.write("const int melody[] = {" + ",".join(map(str, melody)) + "};\n")
            f.write("const int durations[] = {" + ",".join(map(str, durations)) + "};\n\n")
            f.write("int melodyLength = sizeof(melody) / sizeof(melody[0]);\n")
            f.write("void setup() {\n")
            f.write("  for (int i = 0; i < melodyLength; i++) {\n")
            f.write("    tone(4, melody[i], durations[i]);\n")
            f.write("    delay(durations[i] * 1.1);\n")
            f.write("  }\n")
            f.write("}\nvoid loop() {}\n")
        
        # Verificación final
        if os.path.exists(ruta_salida):
            print(f"\n✅ ¡Archivo generado con éxito!")
            print(f"Ubicación: {ruta_salida}")
            print(f"Tamaño: {os.path.getsize(ruta_salida)} bytes")
            print(f"Notas convertidas: {len(melody)}")
            
            # Abrir carpeta en el explorador (Windows)
            try:
                os.startfile(RUTA_BASE)
            except:
                print("\nNota: No se pudo abrir la carpeta automáticamente")
        else:
            print("\n⚠️ Aviso: El archivo no se generó donde se esperaba")

    except PermissionError:
        print(f"\nERROR: No tienes permisos para escribir en: {RUTA_BASE}")
        print("\nSoluciones posibles:")
        print("1. Cierra el Arduino IDE si está abierto")
        print("2. Ejecuta VS Code como administrador")
        print("3. Verifica los permisos de la carpeta")

    except Exception as e:
        print(f"\nERROR al escribir el archivo: {str(e)}")

except Exception as e:
    print(f"\nERROR al procesar el MIDI: {str(e)}")

## ================= INSTRUCCIONES FINALES ================= ##
print("\nInstrucciones para usar el archivo generado:")
print(f"1. Abre la carpeta: {RUTA_BASE}")
print("2. Copia el archivo .ino a tu carpeta de proyectos Arduino")
print("3. Ábrelo con el Arduino IDE y súbelo a tu placa")
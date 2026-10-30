#!/usr/bin/env python3
"""nnue_to_c.py - Convertit un fichier .nnue en code C embarqué et génère son header de configuration.
"""

import argparse
import os
import struct
import sys

def chunk_list(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def find_architecture(file_bytes, input_override=None, hl_override=None):
    if input_override and hl_override:
        p_input = input_override
        p_hl = hl_override
        needed_int16 = p_hl * (p_input + 3) + 1
        needed_bytes = needed_int16 * 2
        if file_bytes < needed_bytes:
            sys.exit(f"Erreur : Le fichier ({file_bytes} octets) est trop petit.")
        header_offset = file_bytes - needed_bytes
        return p_input, p_hl, header_offset

    possible_inputs = [756, 768, 40960, 41024, 45056]
    possible_hls = [16, 32, 64, 128, 256, 512, 1024]

    for p_input in possible_inputs:
        for p_hl in possible_hls:
            needed_int16 = p_hl * (p_input + 3) + 1
            needed_bytes = needed_int16 * 2
            if file_bytes >= needed_bytes:
                header_offset = file_bytes - needed_bytes
                if header_offset in (0, 12, 780) or (header_offset < 2048 and header_offset % 4 == 0):
                    return p_input, p_hl, header_offset

    for p_input in possible_inputs:
        for p_hl in possible_hls:
            needed_bytes = (p_hl * p_input) * 2
            if file_bytes >= needed_bytes:
                header_offset = file_bytes - needed_bytes
                if header_offset < 2048:
                    return p_input, p_hl, header_offset

    return None, None, None

def main():
    ap = argparse.ArgumentParser(description="Convertisseur NNUE vers C avec génération de config")
    ap.add_argument("weights", help="Fichier de poids binaire (.nnue)")
    ap.add_argument("-o", "--output", required=True, help="Fichier C à générer (ex: algorithm/src/mon_reseau.c)")
    ap.add_argument("--input", type=int, help="Forcer la taille d'entrée")
    ap.add_argument("--hl", type=int, help="Forcer la taille de la couche cachée")
    args = ap.parse_args()

    try:
        with open(args.weights, "rb") as f:
            raw_data = f.read()
    except FileNotFoundError:
        sys.exit(f"nnue_to_c: Erreur : Impossible d'ouvrir '{args.weights}'")

    file_bytes = len(raw_data)
    input_size, hl, header_offset = find_architecture(
        file_bytes, input_override=args.input, hl_override=args.hl
    )

    if input_size is None or hl is None:
        sys.exit(f"nnue_to_c: Impossible de déterminer l'architecture du fichier.")

    print(f"nnue_to_c: Analyse de {os.path.basename(args.weights)}")
    print(f"  -> Architecture : INPUT={input_size}, HL={hl}")

    network_data = raw_data[header_offset:]
    expected_int16 = hl * (input_size + 3) + 1
    expected_bytes = expected_int16 * 2

    if len(network_data) < expected_bytes:
        network_data = network_data + b'\x00' * (expected_bytes - len(network_data))
    else:
        network_data = network_data[:expected_bytes]

    total_values = len(network_data) // 2
    vals = struct.unpack("<%dh" % total_values, network_data)

    n_acc = input_size * hl
    acc = vals[:n_acc]
    biases = vals[n_acc:n_acc + hl]
    out_w = vals[n_acc + hl:n_acc + 3 * hl]
    out_b = vals[-1]

    out_dir = os.path.dirname(args.output)
    out_name = os.path.splitext(os.path.basename(args.output))[0]
    
    # 1. Génération du fichier d'en-tête de configuration associé
    config_filename = os.path.join(out_dir, "..", "headers", f"{out_name}_config.h")
    config_filename = os.path.normpath(config_filename)
    
    config_guard = f"__{out_name.upper()}_CONFIG_H__"
    config_lines = [
        "/**",
        f" * \\file {os.path.basename(config_filename)}",
        f" * \\brief Configuration automatique pour le réseau {out_name}.",
        " * \\details GÉNÉRÉ AUTOMATIQUEMENT. NE PAS ÉDITER.",
        " */",
        f"#ifndef {config_guard}",
        f"#define {config_guard}",
        "",
        f"#define NNUE_HL_SIZE {hl}",
        f"#define NNUE_INPUT_SIZE {input_size}",
        "",
        f"#endif",
        ""
    ]
    with open(config_filename, "w", encoding="utf-8") as cf:
        cf.write("\n".join(config_lines))
    print(f"  -> Config générée : {config_filename}")

    # 2. Génération du fichier C de poids
    src_filename = os.path.basename(args.weights)
    lines = [
        "/**",
        f" * \\file {os.path.basename(args.output)}",
        " * \\brief Poids du réseau NNUE (définition de nnue_net).",
        f" * \\details GÉNÉRÉ par tools/nnue_to_c.py depuis {src_filename}",
        " *          NE PAS ÉDITER À LA MAIN.",
        " */",
        "",
        '#include "net_config.h"',
        '#include "nnue_type.h"',
        "",
        "const network nnue_net = {",
        "\t.accumulator_weights = {"
    ]

    for f in range(input_size):
        row = acc[f * hl:(f + 1) * hl]
        lines.append("\t\t{")
        for chunk in chunk_list(row, 12):
            lines.append("\t\t\t" + ", ".join(str(v) for v in chunk) + ",")
        lines.append("\t\t},")
    lines.append("\t},")

    lines.append("\t.accumulator_biases = {")
    for chunk in chunk_list(biases, 12):
        lines.append("\t\t" + ", ".join(str(v) for v in chunk) + ",")
    lines.append("\t},")

    lines.append("\t.output_weights = {")
    for chunk in chunk_list(out_w, 12):
        lines.append("\t\t" + ", ".join(str(v) for v in chunk) + ",")
    lines.append("\t},")

    lines.append(f"\t.output_bias = {out_b},")
    lines.append("};")
    lines.append("")

    with open(args.output, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines))

    print(f"  -> Code C généré  : {args.output}")

if __name__ == "__main__":
    main()
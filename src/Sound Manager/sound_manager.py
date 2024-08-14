import subprocess

video_file = "simulation.avi"
silent_audio = "silent.wav"
final_video = "final_video_with_sound.avi"
collision_frames = [100, 200, 300]  # Example frame numbers
frame_rate = 30  # Example frame rate

# List of sound snippets corresponding to each collision
sound_snippets = ["sound_snippet1.wav", "sound_snippet2.wav", "sound_snippet3.wav"]

# Generate the list of sound overlay commands
overlay_commands = []
for i, frame in enumerate(collision_frames):
    timestamp = int((frame / frame_rate) * 1000)  # Convert frame number to milliseconds
    overlay_commands.append(f"[{i+2}]adelay={timestamp}|{timestamp}[a{frame}]")

# Combine the overlay commands
filter_complex = ";".join(overlay_commands)
amix_inputs = "".join([f"[a{frame}]" for frame in collision_frames])

# Construct the full FFmpeg command
command = [
    "ffmpeg",
    "-i", video_file,
    "-i", silent_audio
]

# Add each sound snippet as input
for snippet in sound_snippets:
    command.extend(["-i", snippet])

command.extend([
    "-filter_complex", f"{filter_complex};[0]{amix_inputs}amix=inputs={len(collision_frames) + 1}",
    final_video
])

# Run the FFmpeg command
subprocess.run(command)

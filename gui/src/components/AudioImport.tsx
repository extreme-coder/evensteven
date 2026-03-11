import { Button } from '@/components/ui/button'
import { useStore } from '@/lib/store'
import type { SongEntry } from '@/lib/store'
import { toast } from 'sonner'

let nextId = 1

export default function AudioImport() {
  const { addSongs } = useStore()

  const handleBrowse = async () => {
    if (!window.electronAPI) return
    const paths = await window.electronAPI.selectFiles()
    if (paths.length === 0) return

    const entries: SongEntry[] = paths.map((p: string) => {
      const filename = p.split('/').pop() ?? p
      const name = filename.replace(/\.[^.]+$/, '')
      return {
        id: `song_${nextId++}`,
        name,
        path: p,
        metadata: {
          filename,
          duration_s: 0,
          sample_rate: 0,
          channels: 0,
          format: p.split('.').pop()?.toLowerCase() ?? 'unknown',
        },
      }
    })

    addSongs(entries)
    toast.success(`Added ${entries.length} file${entries.length > 1 ? 's' : ''}`)

    // Probe for real metadata
    try {
      const results = await window.electronAPI.probeFiles(paths)
      const { updateSongMetadata } = useStore.getState()
      for (let i = 0; i < entries.length; i++) {
        const meta = results[i]
        if (meta && !meta.error) {
          updateSongMetadata(entries[i].id, {
            filename: meta.filename,
            duration_s: meta.duration_s,
            sample_rate: meta.sample_rate,
            channels: meta.channels,
            format: meta.format,
          })
        }
      }
    } catch (err) {
      console.error('Probe failed:', err)
    }
  }

  return (
    <div className="space-y-4">
      <h2 className="text-lg font-semibold">Import Audio Files</h2>

      <div
        className="border-2 border-dashed rounded-lg p-12 text-center cursor-pointer transition-colors border-muted-foreground/25 hover:border-primary/50"
        onClick={handleBrowse}
      >
        <div className="space-y-2">
          <p className="text-lg font-medium">Click to browse audio files</p>
          <p className="text-sm text-muted-foreground">
            Supports WAV, FLAC, and MP3 files
          </p>
        </div>
      </div>

      <div className="text-center">
        <Button variant="outline" onClick={handleBrowse}>
          Browse Files
        </Button>
      </div>
    </div>
  )
}

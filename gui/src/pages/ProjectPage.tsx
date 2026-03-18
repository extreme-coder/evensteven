import { useNavigate } from 'react-router-dom'
import { Button } from '@/components/ui/button'
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import { Separator } from '@/components/ui/separator'
import AudioImport from '@/components/AudioImport'
import { useStore } from '@/lib/store'
import { toast } from 'sonner'

export default function ProjectPage() {
  const navigate = useNavigate()
  const { projectName, songs, removeSong, setStatus, setResults, setError, setProgress } = useStore()

  const handleRunAnalysis = async () => {
    if (songs.length === 0) {
      toast.error('Add at least one audio file before running analysis.')
      return
    }

    if (!window.electronAPI) {
      toast.error('Electron API not available. Please run in Electron.')
      return
    }

    setStatus('running')
    setProgress(null)
    navigate('/analysis')

    try {
      const result = await window.electronAPI.analyzeFiles({
        files: songs.map((s) => s.path),
        projectName,
      })
      if (result.success) {
        setResults(result.data, result.outputDir)
        setStatus('complete')
      } else {
        setError(result.error || 'Analysis failed')
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err))
    }
  }

  const formatDuration = (s: number) => {
    const min = Math.floor(s / 60)
    const sec = Math.floor(s % 60)
    return `${min}:${sec.toString().padStart(2, '0')}`
  }

  return (
    <div className="flex flex-col h-screen">
      <header className="border-b px-6 py-4 flex items-center justify-between">
        <div>
          <Button variant="ghost" size="sm" onClick={() => navigate('/')}>
            &larr; Home
          </Button>
          <span className="ml-4 text-lg font-semibold">{projectName}</span>
        </div>
      </header>

      <div className="flex-1 flex overflow-hidden">
        {/* Song list */}
        <div className="w-80 border-r p-4 overflow-y-auto">
          <h2 className="font-semibold mb-3">Songs ({songs.length})</h2>
          <div className="space-y-2">
            {songs.map((song) => (
              <Card key={song.id} className="p-3">
                <div className="flex items-center justify-between">
                  <div className="min-w-0">
                    <p className="font-medium truncate text-sm">{song.name}</p>
                    <div className="flex gap-2 mt-1">
                      {song.metadata && (
                        <>
                          <Badge variant="secondary" className="text-xs">
                            {song.metadata.format.toUpperCase()}
                          </Badge>
                          <span className="text-xs text-muted-foreground">
                            {formatDuration(song.metadata.duration_s)}
                          </span>
                        </>
                      )}
                    </div>
                  </div>
                  <Button
                    variant="ghost"
                    size="sm"
                    className="text-destructive hover:text-destructive shrink-0 ml-2"
                    onClick={() => removeSong(song.id)}
                  >
                    &times;
                  </Button>
                </div>
              </Card>
            ))}
            {songs.length === 0 && (
              <p className="text-sm text-muted-foreground text-center py-8">
                No songs added yet. Drag and drop audio files to the right.
              </p>
            )}
          </div>
        </div>

        {/* Import area */}
        <div className="flex-1 p-6 overflow-y-auto">
          <AudioImport />
        </div>
      </div>

      <Separator />
      <footer className="px-6 py-4 flex items-center justify-between">
        <span className="text-sm text-muted-foreground">
          {songs.length} song{songs.length !== 1 ? 's' : ''} in setlist
        </span>
        <Button
          size="lg"
          onClick={handleRunAnalysis}
          disabled={songs.length === 0}
        >
          Run Analysis
        </Button>
      </footer>
    </div>
  )
}

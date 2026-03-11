import { Progress } from '@/components/ui/progress'
import { Button } from '@/components/ui/button'
import { Card, CardContent } from '@/components/ui/card'
import { useStore } from '@/lib/store'

export default function ProgressIndicator() {
  const { progress } = useStore()

  const handleCancel = () => {
    if (window.electronAPI) {
      window.electronAPI.cancelAnalysis()
    }
    useStore.getState().setStatus('idle')
  }

  return (
    <Card>
      <CardContent className="py-6 space-y-4">
        <div className="flex items-center justify-between">
          <div>
            <p className="font-medium">Analyzing...</p>
            {progress && (
              <p className="text-sm text-muted-foreground">
                {progress.stage && (
                  <span className="capitalize">{progress.stage.replace('_', ' ')}</span>
                )}
                {progress.song && (
                  <span> — {progress.song}</span>
                )}
              </p>
            )}
          </div>
          <Button variant="outline" size="sm" onClick={handleCancel}>
            Cancel
          </Button>
        </div>
        <Progress value={progress?.percent ?? 0} className="h-2" />
        <p className="text-xs text-muted-foreground text-right">
          {(progress?.percent ?? 0).toFixed(0)}%
        </p>
      </CardContent>
    </Card>
  )
}

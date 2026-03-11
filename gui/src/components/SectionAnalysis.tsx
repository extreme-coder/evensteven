import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import type { SongAnalysis } from '@/lib/store'

interface Props {
  song: SongAnalysis
}

export default function SectionAnalysis({ song }: Props) {
  const sections = song.sections.filter((s) => !s.is_silence)

  if (sections.length === 0) return null

  // Compute per-section masking
  const sectionMetrics = sections.map((sec) => {
    let maskedFrames = 0
    let totalFrames = 0

    for (let i = 0; i < song.balance.timestamps.length; i++) {
      const t = song.balance.timestamps[i]
      if (t >= sec.start_s && t <= sec.end_s) {
        totalFrames++
        if (song.balance.masking_risk[i]) maskedFrames++
      }
    }

    const maskingPct = totalFrames > 0 ? (maskedFrames / totalFrames) * 100 : 0

    return {
      ...sec,
      duration: sec.end_s - sec.start_s,
      maskingPct,
    }
  })

  const maskingColor = (pct: number) => {
    if (pct > 30) return 'text-red-500'
    if (pct > 10) return 'text-yellow-500'
    return 'text-green-500'
  }

  return (
    <Card>
      <CardHeader className="pb-2">
        <CardTitle className="text-base">Section Analysis</CardTitle>
      </CardHeader>
      <CardContent>
        <Table>
          <TableHeader>
            <TableRow>
              <TableHead>Section</TableHead>
              <TableHead>Start</TableHead>
              <TableHead>Duration</TableHead>
              <TableHead className="text-right">Masking Risk</TableHead>
            </TableRow>
          </TableHeader>
          <TableBody>
            {sectionMetrics.map((sec) => (
              <TableRow key={sec.label}>
                <TableCell className="font-medium">{sec.label}</TableCell>
                <TableCell>
                  {Math.floor(sec.start_s / 60)}:
                  {Math.floor(sec.start_s % 60).toString().padStart(2, '0')}
                </TableCell>
                <TableCell>{sec.duration.toFixed(1)}s</TableCell>
                <TableCell className={`text-right font-medium ${maskingColor(sec.maskingPct)}`}>
                  {sec.maskingPct.toFixed(0)}%
                </TableCell>
              </TableRow>
            ))}
          </TableBody>
        </Table>
      </CardContent>
    </Card>
  )
}
